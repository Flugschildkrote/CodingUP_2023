#include <cstdlib>
#include <iostream>
#include <anubis/Anubis.hpp>
#include <anubis/Tree.hpp>
#include <anubis/net/Http.hpp>
#include <cassert>
#include <stb_image.h>
#include <stb_image_write.h>
#include <fstream>
#include <thread>
#include <mutex>
#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <execution>
#include <filesystem>
#include <sndfile.hh>
#include <map>
#include <future>
#include <anubis/sound/SoundFile.hpp>
#include <anubis/util/Timer.hpp>
#include <anubis/util/ThreadPool.hpp>
#include <anubis/algo/Async.hpp>

#ifdef min
#undef min
#endif // min



template <typename T>
T myPow(T value, size_t pow) {
    if (value == 0)  return T(1);
    T result = value;
    for (size_t i = 1; i < pow; ++i) {
        result *= value;
    }

    return result;
}

uint32_t CountBits(uint64_t val)
{
    uint32_t result = 0;
    for (uint32_t i = 0; i < sizeof(val) * 8; ++i) {
        result += static_cast<bool>(val & 0x01);
        val >>= 1;
    }

    return result;
}

void PrintBitsIndex(uint64_t val)
{
    for (uint32_t i = 0; i < sizeof(val) * 8; ++i) {
        if (val & 0x01) {
            std::cout << (i + 1) << ", ";
        }
        val >>= 1;
    }
    std::cout << std::endl;
}


int main(void)
{
    anubis::ThreadPool pool;
    try {
        namespace fs = std::filesystem;
        std::cout << fs::current_path() << std::endl;
        std::string path = "./data/ref_musics";
        auto itt = fs::directory_iterator(path);

        using sampleType_t = double;

        using AudioFileData_t = std::vector<sampleType_t>;
        std::vector<anubis::AudioFile> audiosData;

        for (const auto& entry : fs::directory_iterator(path))
        {
            std::string filePath = entry.path().string();
            audiosData.emplace_back(filePath);
        }

        const uint32_t samplePerHalfSecond = audiosData[0].getSampleRate() / 2;
        const uint32_t numSamples = audiosData[0].getNumFrames();
        uint32_t numFiles = static_cast<uint32_t>(audiosData.size());

        // generate mixed data 
        // On garde seulement ceux ou il y a exactement 7 bits à 1
        uint32_t numMix = myPow<uint32_t>(2, numFiles);

        //std::vector<std::vector<sampleType_t>> mixedAudios;
        std::map<uint32_t, std::vector<sampleType_t>> mixedAudios;
        //mixedAudios.reserve(nu)

        std::cout << "Generating " << numMix << " mixes.." << std::endl;

        size_t selectedNumSamples = 80000;

        std::mutex lock;
        auto genIteration = [&lock, &mixedAudios, selectedNumSamples, numFiles, &audiosData](uint64_t i) {
            if (CountBits(i) != 7) {
                return;
            }

            std::vector<sampleType_t> currentMix(selectedNumSamples);
            uint32_t numFilesInMix = 0;

            for (size_t refAudioId = 0; refAudioId < numFiles; ++refAudioId) {
                if ((i & (uint64_t(1) << refAudioId)) > 0) {
                    const anubis::AudioFile& currentAudioToMix = audiosData[refAudioId];
                    // Add the audio file to the samples
                    numFilesInMix++;
                    anubis::MemoryView<const double> currentChannelToMix = currentAudioToMix.getChannel(0);
                    for (size_t ii = 0; ii < selectedNumSamples; ++ii) {
                        currentMix[ii] += currentChannelToMix[ii]; // *2 car on est en stéréo entremellé
                    }
                }
            }

            assert(numFilesInMix == 7);
            for (size_t ii = 0; ii < selectedNumSamples; ++ii) {
                currentMix[ii] /= sampleType_t(numFilesInMix);
            }

            std::lock_guard lk(lock);
            mixedAudios[i] = std::move(currentMix);
        };

   
        //mixedAudios[0].resize(selectedNumSamples);
        //std::fill(mixedAudios[0].begin(), mixedAudios[0].end(), sampleType_t(0));
        anubis::Timer timer;

        timer.start();
        anubis::parallelForN(pool, genIteration, numMix, 1);
        timer.stop();
        std::cout << "Parallel done in " << timer.getSeconds() << std::endl;

       
        std::cout << "Mixes with 7 samples=" << mixedAudios.size() << std::endl;

        // Ici l'audio est mixé
        // Try to find a match

        SndfileHandle mysteryFile("data/cacophonie.wav", SFM_READ);
        std::vector<sampleType_t> samples(mysteryFile.channels() * mysteryFile.frames());
        mysteryFile.readf(samples.data(), selectedNumSamples);


        std::map<uint32_t, sampleType_t> diffRatio;
        for(auto it=mixedAudios.begin(); it != mixedAudios.end(); ++it) 
        {
            sampleType_t& diff = diffRatio[it->first];
            for (size_t ii = 0; ii < selectedNumSamples; ++ii) {
                diff += std::abs(it->second[ii] - samples[ii*2]);
            }
            diff /= static_cast<double>(selectedNumSamples);
            // std::cout << "[" << i << "]" << " match=" << diff << std::endl;
        }

        // find the closest match 
        sampleType_t closestMatch = diffRatio.begin()->first;
        size_t closestIndex = 0;
        for (auto it = diffRatio.begin(); it != diffRatio.end(); ++it) {

            if (it->second < closestMatch) {
                closestMatch = it->second;
                closestIndex = it->first;
            }
           // if (diffRatio[i] < 10e-3) {
               // std::cout << "May match with " << i << " : " << diffRatio[i] << std::endl;
           // }
        }
        std::cout << "May match with " << closestIndex << " : " << closestMatch << std::endl;
        PrintBitsIndex(closestIndex);

    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}