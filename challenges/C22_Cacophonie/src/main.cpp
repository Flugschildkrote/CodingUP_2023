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

template <typename T>
T myPow(T value, size_t pow) {
    if (value == 0)  return T(1);
    T result = value;
    for (size_t i = 1; i < pow; ++i) {
        result *= value;
    }

    return result;
}


int main(void)
{
    try {
        namespace fs = std::filesystem;
        std::cout << fs::current_path() << std::endl;
        std::string path = "./data/ref_musics";
        auto itt = fs::directory_iterator(path);

        using sampleType_t = double;

        using AudioFileData_t = std::vector<sampleType_t>;
        std::vector<AudioFileData_t> audiosData;
        uint32_t numSamples;
        uint32_t samplePerSecond;
        uint32_t numFiles = 0;

        for (const auto& entry : fs::directory_iterator(path))
        {
            numFiles += 1;
            SndfileHandle handle(entry.path().c_str(), SFM_READ);
            if (handle)
            {
                AudioFileData_t& fileData = audiosData.emplace_back(handle.frames() * handle.channels());
                if (handle.channels() != 2) {
                    throw std::runtime_error("Audio file was not stereo");
                }

                numSamples = handle.frames();
                samplePerSecond = handle.samplerate();
                handle.readf(fileData.data(), fileData.size());
            }
            else {
                throw std::runtime_error("Failed to read audioFile");
            }
        }

        const uint32_t samplePerHalfSecond = samplePerSecond / 2;

        // generate mixed data 
        uint32_t numMix = myPow<uint32_t>(2, numFiles);
        std::vector<std::vector<sampleType_t>> mixedAudios(numMix);

        std::cout << "Generating " << numMix << " mixes.." << std::endl;

        size_t selectedNumSamples = 20000;
        mixedAudios[0].resize(selectedNumSamples);
        std::fill(mixedAudios[0].begin(), mixedAudios[0].end(), sampleType_t(0));
        for (size_t i = 1; i < numMix; ++i)
        {
            std::vector<sampleType_t>& currentMix = mixedAudios[i];
            currentMix.resize(selectedNumSamples);
            uint32_t numFilesInMix = 0;

            for (size_t refAudioId = 0; refAudioId < numFiles; ++refAudioId) {
                if ((i & (1 << refAudioId)) > 0) {
                    const std::vector<sampleType_t>& currentAudioToMix = audiosData[refAudioId];
                    // Add the audio file to the samples
                    numFilesInMix++;
                    for (size_t ii = 0; ii < selectedNumSamples; ++ii) {
                        currentMix[ii] += currentAudioToMix[ii * 2]; // *2 car on est en stéréo entremellé
                    }
                }
            }

            for (size_t ii = 0; ii < selectedNumSamples; ++ii) {
                currentMix[ii] /= sampleType_t(numFilesInMix);
            }
        }


        // Ici l'audio est mixé

    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;

    /*std::vector<std::string> inFiles = {
        "data/0_tetris.wav",
        "data/1_bubble_bobble.wav",
        "data/2_super_mario_kart.wav",
        "data/3_pacmania.wav",
        "data/4_super_mario_bros.wav"
    };

    using sampleType_t = double;
    std::vector<AudioFile<sampleType_t>> audioData(inFiles.size());
    const uint32_t numSamples = 661500;
    const uint32_t samplePerSecond = 44100;
    const uint32_t samplePerHalfSecond = samplePerSecond / 2;

    for (size_t i = 0; i < inFiles.size(); ++i) {
        audioData[i] = AudioFile<sampleType_t>(inFiles[i]);
        audioData[i].printSummary();
    }

    AudioFile<sampleType_t> encodedFile("data/message_code.wav");
    encodedFile.printSummary();
    const std::vector<sampleType_t>& samples = encodedFile.samples[0];

    // generate reference data 
    uint32_t numMix = myPow<uint32_t>(2, inFiles.size());
    std::vector<std::vector<sampleType_t>> mixedAudios(numMix);

    mixedAudios[0].resize(numSamples);
    std::fill(mixedAudios[0].begin(), mixedAudios[0].end(), sampleType_t(0));
    for (size_t i = 1; i < numMix; ++i) {
        std::vector<sampleType_t>& currentMix = mixedAudios[i];
        currentMix.resize(numSamples);
        uint32_t numFilesInMix = 0;

        for (size_t refAudioId = 0; refAudioId < inFiles.size(); ++refAudioId) {
            if ((i & (1 << refAudioId)) > 0) {
                const std::vector<sampleType_t>& currentAudioToMix = audioData[refAudioId].samples[0];
                // Add the audio file to the samples
                numFilesInMix++;
                for (size_t ii = 0; ii < currentMix.size(); ++ii) {
                    currentMix[ii] += currentAudioToMix[ii];
                }
            }
        }

        for (size_t ii = 0; ii < numSamples; ++ii) {
            currentMix[ii] /= sampleType_t(numFilesInMix);
        }
    }

    std::string message = "";
    for (int i = 0; i < 30; ++i)
    {
        size_t sampleIndex = i * samplePerHalfSecond;
        auto startIt = samples.begin() + sampleIndex;
        auto endIt = startIt + samplePerHalfSecond;
        //sampleType_t sampleSum = std::accumulate(startIt, endIt, 0);

        //std::cout << "[" << static_cast<float>(i) / 2.0f << "-" << static_cast<float>(i + 1) / 2.0f << "]s :\n";
        std::vector<sampleType_t> diffRatio(mixedAudios.size());
        for (size_t i = 0; i < diffRatio.size(); ++i) 
        {
            sampleType_t& diff = diffRatio[i];
            for (size_t ii = 0; ii < samplePerHalfSecond; ++ii) {
                diff += std::abs(mixedAudios[i][sampleIndex + ii] - samples[sampleIndex + ii]);
            }
            diff /= static_cast<double>(samplePerHalfSecond);
           // std::cout << "[" << i << "]" << " match=" << diff << std::endl;
        }

        // find the closest match 
        sampleType_t closestMatch = diffRatio[0];
        char closestIndex = 0;
        for (size_t i = 1; i < diffRatio.size(); ++i) {

            if (diffRatio[i] < closestMatch) {
                closestMatch = diffRatio[i];
                closestIndex = (char)i;
            }
        }

        message += (closestIndex == 27) ? ' ' : ('A' + closestIndex - 1);
       // std::cout << "[" << static_cast<float>(i) / 2.0f << "-" << static_cast<float>(i + 1) / 2.0f << "]s : " << closestIndex << std::endl;

    }

    std::cout << message << std::endl;
    //audioData[0].


    //std::cout << "Audio length=" << audio.getLengthInSeconds() << "s" << std::endl;
    */
    return EXIT_SUCCESS;
}