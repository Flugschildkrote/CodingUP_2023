#include <cstdlib>
#include <iostream>
#include <anubis/Anubis.hpp>
#include <anubis/Tree.hpp>
#include <anubis/net/Http.hpp>
#include <cassert>
#include <stb_image.h>
#include <stb_image_write.h>
#include <fstream>

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

int main(void)
{
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
        std::string_view base64Data(data.begin() + endLine + 1, data.end()); // remove the -- Anubis -- header sent by the server
        std::cout << "base64 data length=" << base64Data.length() << std::endl;
        std::vector<uint8_t> pixels = base64Decode(base64Data);
        std::string_view pixelView((const char*)pixels.data(), pixels.size());
        //std::cout << pixelView << std::endl;
        //std::cout << pixels.size() << std::endl;
        std::ofstream pngout("out.png", std::ios::binary);
        pngout.write((const char*)pixels.data(), pixels.size());

       // stbi_write_bmp("Out.bmp", 40, 20, 3, pixels.data());
    }
    else {
        std::cout << "error while reading : " << result << std::endl;
    }
    anubis::dummy();
    return EXIT_SUCCESS;
}