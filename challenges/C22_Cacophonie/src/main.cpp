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

class ThreadPool
{
public:
    /* 
     * @brief Creates a thread pool with a fiex amount of threads
     * @param numThreads The number of threads in the pool. 0 mean automatic (ie. The maximum number of concurrent threads allowed by the machine)
     * @param threadMultiplier The pool will start numThreads * threadMultiplier. When using automatic mode, it may be a good idea to add more threads for better loadBalancing
     */
    ThreadPool(uint32_t numThreads = 0, uint32_t threadMultiplier = 1)
    {
        numThreads = numThreads ? numThreads : std::thread::hardware_concurrency();
        numThreads *= threadMultiplier;

        ThreadPool& pool = *this;
        auto workerFnc = [&pool] {

            std::unique_lock lk(pool.m_MainMutex);
            // here the mutex is aquired
            do
            {
                //  Wait for a job if the queue is empty. Will be woken up when pool request termination
                if (pool.m_WorkerThreads.empty()) {
                    pool.m_TasksConditionVariable.wait(lk, [&pool]() { return !pool.m_TasksQueue.empty() || pool.m_ExitRequested; });
                }

                // Exit if needed.
                if (pool.m_ExitRequested) {
                    break;
                }

                // Pop the job and execute it
                std::function<void(void)> fnc = std::move(pool.m_TasksQueue.front());
                pool.m_TasksQueue.pop();

                lk.unlock();
                fnc();
                lk.lock();
            } while (true);

        };

        // Start worker threads
        m_WorkerThreads.resize(numThreads);
        for (std::thread& t : m_WorkerThreads) {
            t = std::thread(workerFnc);
        }
    }

    template <typename T_RESULT, typename T_TASK>
    std::future<T_RESULT> pushTask(T_TASK&& op) {
        std::promise<T_RESULT> promise;

        m_TasksQueue.push(
            []() {

            }
        )
    }


private:

    void terminateThreads(void)
    {
        {
            std::lock_guard lk(m_MainMutex);
            m_ExitRequested = true;
        }
        m_TasksConditionVariable.notify_all();

        for (std::thread& t : m_WorkerThreads) {
            t.join();
        }
    }

    std::vector<std::thread> m_WorkerThreads;
    std::queue<std::function<void(void)>> m_TasksQueue;
    std::condition_variable m_TasksConditionVariable;
    std::mutex m_MainMutex;
    bool m_ExitRequested;

    //std::stack<uint32
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