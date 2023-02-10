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

template <typename T>
T myPow(T x, T p) {
    T i = 1;
    for (T j = 1; j <= p; j++)  i *= x;
    return i;
}

constexpr size_t KEY_BRUTEFORCE_COUNT = 256;
std::array<stbi_uc*, KEY_BRUTEFORCE_COUNT> decoded_pictures;

int32_t getNextKey(int tid) {
    static std::mutex myMutex;
    static int pictureCounter = 0;

    std::lock_guard<std::mutex> lock(myMutex);

    std::cout << "Thread [" << tid << "] working on key " << pictureCounter << std::endl;

    if (pictureCounter == KEY_BRUTEFORCE_COUNT) {
        return KEY_BRUTEFORCE_COUNT;
    }

    int32_t key = pictureCounter;
    pictureCounter++;
    return key;
}

void floatVersion()
{
    // Loading the picture
    int w, h, comp;
    //stbi_set_flip_vertically_on_load(true);
    const stbi_uc* in_encoded_picture = stbi_load("data/dungeons_portal_enc.png", &w, &h, &comp, 0);
    stbi_uc* reference_picture = new stbi_uc[w * h];
    const stbi_uc* reference_encoded_picture_no_xor = reference_picture;

    // Performing float power operations and filling array
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x)
        {
            double xd = x;
            double yd = y;
            double val = std::pow(xd, 3) + std::pow(yd, 7);
            double modulatedVal = std::round(std::fmod(val, 256));
            int irawValue = modulatedVal;
            stbi_uc referenceValue = (stbi_uc)irawValue;
            reference_picture[y * w + x] = referenceValue;
        }
    }

    // allocate the result pictures
    for (int i = 0; i < decoded_pictures.size(); ++i) {
        decoded_pictures[i] = new stbi_uc[w * h];
    }

    auto threadWorker = [w, h, reference_encoded_picture_no_xor, in_encoded_picture](int tid) {
        int32_t key = getNextKey(tid);
        while (key != KEY_BRUTEFORCE_COUNT) {
            stbi_uc* resultPicture = decoded_pictures[key];

            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x)
                {
                    int32_t pixelIndex = y * w + x;
                    bool keyMatch = ((reference_encoded_picture_no_xor[pixelIndex] ^ key) == in_encoded_picture[pixelIndex]);
                    resultPicture[pixelIndex] = keyMatch ? 255 : 0;
                }
            }

            key = getNextKey(tid);
        }
    };

    // launching threads
    int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads(numThreads);
    for (int tid = 0; tid < threads.size(); ++tid) {
        std::thread& t = threads[tid];
        t = std::thread(std::bind(threadWorker, tid));
    }
    std::cout << "Waiting for jobs end" << std::endl;
    for (std::thread& t : threads) {
        t.join();
    }
    //stbi_set_flip_vertically_on_load(true);
    stbi_uc* new_image = new stbi_uc[w * h];

    for (int key = 0; key < KEY_BRUTEFORCE_COUNT; ++key) {
        std::string fileName = "data/decoded" + std::to_string(key) + ".bmp";
        stbi_write_bmp(fileName.c_str(), w, h, 1, decoded_pictures[key]);
        std::cout << "Writing [" << key + 1 << "/" << KEY_BRUTEFORCE_COUNT << "]" << std::endl;
    }
}





void intMethod() {
    using valType = boost::multiprecision::int128_t;

    int w, h, comp;
    //stbi_set_flip_vertically_on_load(true);
    const stbi_uc* in_encoded_picture = stbi_load("data/dungeons_portal_enc.png", &w, &h, &comp, 0);
    stbi_uc* reference_picture = new stbi_uc[w * h];
    const stbi_uc* reference_encoded_picture_no_xor = reference_picture;

    // Performing float power operations and filling array
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x)
        {
 
            valType val = myPow<valType>(x, 3) + myPow<valType>(y, 7);
            valType modulatedVal = val % 256;
            stbi_uc referenceValue = (stbi_uc)modulatedVal;
            reference_picture[y * w + x] = referenceValue;
        }
    }

    // allocate the result pictures
    for (int i = 0; i < decoded_pictures.size(); ++i) {
        decoded_pictures[i] = new stbi_uc[w * h];
    }

    auto threadWorker = [w, h, reference_encoded_picture_no_xor, in_encoded_picture](int tid) {
        int32_t key = getNextKey(tid);
        while (key != KEY_BRUTEFORCE_COUNT) {
            stbi_uc* resultPicture = decoded_pictures[key];

            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x)
                {
                    int32_t pixelIndex = y * w + x;
                    bool keyMatch = ((reference_encoded_picture_no_xor[pixelIndex] ^ key) == in_encoded_picture[pixelIndex]);
                    resultPicture[pixelIndex] = keyMatch ? 255 : 0;
                }
            }

            key = getNextKey(tid);
        }
    };

    // launching threads
    int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads(numThreads);
    for (int tid = 0; tid < threads.size(); ++tid) {
        std::thread& t = threads[tid];
        t = std::thread(std::bind(threadWorker, tid));
    }
    std::cout << "Waiting for jobs end" << std::endl;
    for (std::thread& t : threads) {
        t.join();
    }
    //stbi_set_flip_vertically_on_load(true);
    stbi_uc* new_image = new stbi_uc[w * h];

    for (int key = 0; key < KEY_BRUTEFORCE_COUNT; ++key) {
        std::string fileName = "data/decoded" + std::to_string(key) + ".bmp";
        stbi_write_bmp(fileName.c_str(), w, h, 1, decoded_pictures[key]);
        std::cout << "Writing [" << key + 1 << "/" << KEY_BRUTEFORCE_COUNT << "]" << std::endl;
    }
}

int main(void)
{
    //floatVersion();
    intMethod();
    return EXIT_SUCCESS;
}