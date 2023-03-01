#include <cstdlib>
#include <iostream>
#include <anubis/Anubis.hpp>
#include <anubis/Tree.hpp>
#include <anubis/net/Http.hpp>
#include <cassert>
#include <stb_image.h>
#include <stb_image_write.h>
#include <fstream>
#include <anubis/util/Iterator.hpp>
#include <anubis/util/MemoryView.hpp>
#include <anubis/image/Image.hpp>
#include <anubis/image/ImageView.hpp>

#ifdef min
#undef min
#endif // min

size_t stringReader(std::string_view inData, std::string& outStr) {
    outStr = std::string(inData);
    return outStr.size();
}

std::vector<uint8_t> base64Decode(std::string_view data_base64) {
    
    assert(((data_base64.length() % 4) == 0) && "Base64 data must be a multiple of 4 bytes in length");

    auto charToValue = [](char v) -> uint8_t {
        if (v >= 'A' && v <= 'Z')
            return v - 'A';

        if (v >= 'a' && v <= 'z')
            return v - 'a' + 0b011010;

        if (v >= '0' && v <= '9')
            return v - '0' + 0b110100;

        if (v == '+')
            return 0b111110;

        if (v == '/')
            return 0b111111;

        return 0;
    };

    std::vector<uint8_t> binaryResult;
    //result.reserve(data_base64.length()); // a bit too much but is ok
    auto it = data_base64.begin();
    for (it; it < data_base64.end()-4; it += 4)
    {
        // decoding is done using 4 characters (=3 bytes)
        uint32_t data = 0; // 6 bits

        for (int32_t i = 0; i < 4; ++i) {
            data <<= 6;
            data |= charToValue(*(it+i)); // 6 bits
        }

        binaryResult.push_back(static_cast<uint8_t>(data >> 16));
        binaryResult.push_back(static_cast<uint8_t>(data >> 8));
        binaryResult.push_back(static_cast<uint8_t>(data));
    }

    if (*(it + 2) == '=') { // Decode only one byte (XX==)
        uint8_t data = (*it);
        data <<= 2;
        data |= (*(it + 1) >> 4);
        binaryResult.push_back(data);

    }else if (*(it + 3) == '=') { // Decode only two byte (XXX=)
        uint16_t data = (*it);
        data <<= 6;
        data |= (*(it + 1));
        data <<= 6;
        data |= (*(it + 2) >> 2);

        binaryResult.push_back(static_cast<uint8_t>(data >> 8));
        binaryResult.push_back(static_cast<uint8_t>(data));
    }
    else { // Decode full three bytes (XXXX)
        uint32_t data = 0; // 6 bits
        for (int32_t i = 0; i < 4; ++i) {
            data <<= 6;
            data |= charToValue(*(it+i)); // 6 bits
        }

        binaryResult.push_back(static_cast<uint8_t>(data >> 16));
        binaryResult.push_back(static_cast<uint8_t>(data >> 8));
        binaryResult.push_back(static_cast<uint8_t>(data));
    }

    return binaryResult;
} 

struct RGB_Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    bool any(void) const noexcept { return bool(r | g | b); }
    bool operator&(const RGB_Pixel& other) { return any() && other.any(); }
};

using Image = anubis::Image;

template <typename T>
using ImageView = anubis::ImageView<T>;

struct InvadersInfo {
    uint32_t tileWidth;
    uint32_t tileHeight;
    Image invadersImage;
    std::vector<ImageView<RGB_Pixel>> invaders;
} invadersInfo;

//std::map<char, 

void readSpaceInvadersReferenceFile(void) {

    invadersInfo.invadersImage = Image("data/invaders_ref.png", 3);
    Image& refInvaders = invadersInfo.invadersImage;

    const uint32_t w = refInvaders.getWidth();
    const uint32_t h = refInvaders.getHeight();
    const uint8_t* pixels = refInvaders.data();
    int pixelScale = w;
    int pixelCount = 0;
    int tileHeight = 0;




    // search for pixel scale
    for (uint32_t y = 0; y < h; ++y)
    {
        for (uint32_t x = 0; x < w; ++x)
        {
            uint32_t pixelPos = (y * w + x)*3;
            uint32_t pixelTotal = pixels[pixelPos] + pixels[pixelPos + 1] + pixels[pixelPos + 2];

            if (pixelTotal > 0 && pixelTotal < (255 * 3)) {
                tileHeight = y;
                goto end_scan;
            }

            if (pixelTotal > 0) {
                pixelCount++;
            }
            else if(pixelCount > 0){

                pixelScale = std::min<int>(pixelCount, pixelScale);
                pixelCount = 0;
            }
        }
    }
end_scan:


    int numAliens = 9;
    tileHeight /= pixelScale;
    int tileWidth = w / pixelScale / numAliens;


    std::vector<ImageView<RGB_Pixel>> &alienReferences = invadersInfo.invaders;
    for (uint32_t i = 0; i < numAliens; ++i) {
        alienReferences.push_back(refInvaders.getView<RGB_Pixel>(i* tileWidth, 0, tileWidth, tileHeight, pixelScale));
    }
    invadersInfo.tileWidth = tileWidth;
    invadersInfo.tileHeight = tileHeight;


    /*ImageView<RGB_Pixel> view = refInvaders.getView<RGB_Pixel>(0, 0, tileWidth, tileHeight, pixelScale);
    for (auto row : view) {
        for (const auto& pixel : row) {
            std::cout << pixel.any();
        }
        std::cout << std::endl;
    }*/

    std::cout << "Pixel size=" << pixelScale << ", " << tileWidth << "x" << tileHeight << std::endl;

}

int main(void)
{
    try {
        readSpaceInvadersReferenceFile();
        //return EXIT_SUCCESS;
        auto printOperator = [](const int& value, size_t accumulator) {
            std::cout << value << "-" << std::endl;
            return 0;
        };

        anubis::Tree<int> myTree(0);
        anubis::Tree<int>* t10 = myTree.addNode(10);
        anubis::Tree<int>* t20 = t10->addNode(20);
        anubis::Tree<int>* t21 = t10->addNode(21);
        anubis::Tree<int>* t11 = myTree.addNode(11);
        anubis::Tree<int>* t22 = t11->addNode(22);
        anubis::Tree<int>* t23 = t11->addNode(23);

        std::cout << myTree.size() << std::endl;
        myTree.traverse<int>(printOperator, 0, 2);

        // https://pydefis.callicode.fr/defis/C22_Invaders01/post/Anubis29/57f04
        anubis::net::HTTPSession httpSession("pydefis.callicode.fr", true);

        std::string data;
        CURLcode result = httpSession.get<std::string>("/defis/C22_Invaders01/get/Anubis29/57f04", data, stringReader);

        if (result == CURLE_OK) {
            std::cout << data << std::endl;
            size_t endLine = data.find_first_of('\n');
            std::string_view myKey(data.begin() , data.begin() + endLine); // remove the -- Anubis -- header sent by the server
            std::string_view base64Data(data.begin() + endLine + 1, data.end()); // remove the -- Anubis -- header sent by the server
            std::cout << "base64 data length=" << base64Data.length() << std::endl;
            std::vector<uint8_t> pixels = base64Decode(base64Data);
            std::string_view pixelView((const char*)pixels.data(), pixels.size());

            Image mysteryPicture(pixels.data(), pixels.size(), 3);

            //std::cout << mysteryPicture.getChannels() << std::endl;
            // 
            // 
            //std::cout << pixels.size() << std::endl;

            std::string invadersString = "";
            for (int mysteryInvaderId = 0; mysteryInvaderId < (6 * 4); ++mysteryInvaderId) 
            {
                size_t x = (mysteryInvaderId % 6) * invadersInfo.tileWidth;
                size_t y = (mysteryInvaderId / 6) * invadersInfo.tileHeight;

                ImageView<RGB_Pixel> mysteryInvader = mysteryPicture.getView<RGB_Pixel>(x, y, invadersInfo.tileWidth, invadersInfo.tileHeight, 4);


                for (int refInvaderId = 0; refInvaderId < invadersInfo.invaders.size(); ++refInvaderId)
                {
                    ImageView<RGB_Pixel>& refInvader = invadersInfo.invaders[refInvaderId];
                    bool match = true;
                    for (size_t y = 0; y < refInvader.size(); ++y)
                    {
                        auto refRow = refInvader[y];
                        auto firstInvaderRow = mysteryInvader[y];

                        for (size_t x = 0; x < refRow.size() && match; ++x)
                        {
                            match = refRow[x].any() == firstInvaderRow[x].any();
                        }
                    }

                    if (match) {
                        invadersString += char('A' + refInvaderId);
                        //std::cout << char('A' + refInvaderId);
                    }
                }

                /*for (auto row : mysteryInvader) {
                    for (const auto& pixel : row) {
                        std::cout << pixel.any();
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl;*/
            }
            std::cout << std::endl;

            std::string inData;
            std::function<std::string(const std::string&)> writer = [](const std::string& str) -> std::string { return str; };

            std::string jsonRes = "sig=" + std::string(myKey) + "&rep=" + invadersString;
            /*Json::Value root;
            root["sig"] = std::string(myKey);
            root["rep"] = invadersString;*/

            CURLcode result = httpSession.post<std::string>("/defis/C22_Invaders01/post/Anubis29/57f04", jsonRes, writer, inData, stringReader, "application/x-www-form-urlencoded");
            std::cout << "Result=" << result << "\nData = \"" << inData << "\"" << std::endl;
            std::ofstream pngout("data/out.png", std::ios::binary);
            pngout.write((const char*)pixels.data(), pixels.size());

            // stbi_write_bmp("Out.bmp", 40, 20, 3, pixels.data());
        }
        else {
            std::cout << "error while reading : " << result << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}