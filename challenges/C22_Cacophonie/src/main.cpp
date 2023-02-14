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


template <typename T>
class MemoryView 
{
public:

    template <typename TT>
    class Iterator_t
    {
    public:
        Iterator_t(TT& mv, size_t cursor) {

        }

    private:
        TT* m_MemoryView;
        size_t m_Cursor;
    };

    size_t size(void) const noexcept {
        const size_t removedElements = (m_VirtualElementStartOffset + m_ElementSize - 1) / m_VirtualElementStride;
        const size_t numElements = (m_RawBufferSizeBytes / m_VirtualElementStride;
        return (numElements - removedElements);
    }

    const T& operator[](size_t index) const {
        size_t byteOffset = m_VirtualElementStartOffset + (m_VirtualElementStride * index);
        size_t endOffset = byteOffset + m_ElementSize;

        if (endOffset > m_RawBufferSizeBytes) {
            throw std::runtime_error("MemoryViex: index out of bounds");
        }

        void* dataAddress = m_RawBuffer + byteOffset;
        return *reinterpret_cast<T*>(dataAddress);
    }

    T& operator[](size_t index) {
        size_t byteOffset = m_VirtualElementStartOffset + (m_VirtualElementStride * index);
        size_t endOffset = byteOffset + m_ElementSize;

        if (endOffset > m_RawBufferSizeBytes) {
            throw std::runtime_error("MemoryViex: index out of bounds");
        }

        void* dataAddress = m_RawBuffer + byteOffset;
        return *reinterpret_cast<T*>(dataAddress);
    }


private:
    void* m_RawBuffer;
    size_t m_RawBufferSizeBytes;
    size_t m_VirtualElementStartOffset; // Required: 
    size_t m_VirtualElementStride; // Required : m_RawBufferSizeBytes % stride == 0
    static constexpr size_t m_ElementSize = sizeof(T);
};


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
        // On garde seulement ceux ou il y a exactement 7 bits à 1
        uint32_t numMix = myPow<uint32_t>(2, numFiles);



        //std::vector<std::vector<sampleType_t>> mixedAudios;
        std::map<uint32_t, std::vector<sampleType_t>> mixedAudios;
        //mixedAudios.reserve(nu)

        std::cout << "Generating " << numMix << " mixes.." << std::endl;

        size_t selectedNumSamples = 40000;
        //mixedAudios[0].resize(selectedNumSamples);
        //std::fill(mixedAudios[0].begin(), mixedAudios[0].end(), sampleType_t(0));
        for (uint64_t i = 1; i < numMix; ++i)
        {
            if (CountBits(i) != 7) {
                continue;
            }

            std::vector<sampleType_t>& currentMix = mixedAudios[i];
            currentMix.resize(selectedNumSamples);
            uint32_t numFilesInMix = 0;

            for (size_t refAudioId = 0; refAudioId < numFiles; ++refAudioId) {
                if ((i & (uint64_t(1) << refAudioId)) > 0) {
                    const std::vector<sampleType_t>& currentAudioToMix = audiosData[refAudioId];
                    // Add the audio file to the samples
                    numFilesInMix++;
                    for (size_t ii = 0; ii < selectedNumSamples; ++ii) {
                        currentMix[ii] += currentAudioToMix[ii * 2]; // *2 car on est en stéréo entremellé
                    }
                }
            }

            assert(numFilesInMix == 7);
            for (size_t ii = 0; ii < selectedNumSamples; ++ii) {
                currentMix[ii] /= sampleType_t(numFilesInMix);
            }
        }

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