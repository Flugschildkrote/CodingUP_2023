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

/*
template <typename T>
class VectorView 
{
public:
    using value_type = T;
public:
    VectorView(T* pData, size_t size)
        : m_Data(pData), m_Size(size)
    {

    };

    size_t size(void) const noexcept { return m_Size; }


    T* getBuffer(void) noexcept { return m_Data; }
    const T* getBuffer(void) const noexcept { return m_Data; }

    T& operator[](size_t index) noexcept {
        assert((index < m_Size) && "Index out of bounds"); 
        return m_Data[index];
    };

    const T& operator[](size_t index) const noexcept {
        assert((index < m_Size) && "Index out of bounds");
        return m_Data[index];
    };

private:
    T* m_Data;
    size_t m_Size;


    template <typename T_IT>
    class Iterator {
    public:

    private:
        VectorView
    };
};
*/


/*struct Pixel
{
    char r;
    char g;
    char b;

    /*bool operator&&(const Pixel& other) const {
        return (r & other.r) 
    }
};*/

/*
class PictureRGBA {

    //std::unique_ptr<
    const Pixel& get

};*/
class Image;

template <typename T>
class ImageView
{
    using RawBuffer_t = std::conditional_t<std::is_const_v<T>, const void*, void*>;
    using Image_t = std::conditional_t<std::is_const_v<T>, const Image, Image>;

public:

    using Iterator = anubis::GenericIterator<ImageView<T>, anubis::MemoryView<T>>;
    using ConstIterator = anubis::GenericIterator<ImageView<const T>, anubis::MemoryView<const T>>;

public:

    /*
     * Construct an image view starting at (x,y) and size of (w, h)
     * All parameters are expressed in virtual space (a virtual pixel is composed of (virtualScale x virtualScale) real pixels
     */
    ImageView(Image_t& img, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t virtualScale)
        : m_Image(img), m_VirtualPixelWidth(virtualPxW), m_VirtualPixelHeight(virtualPxH)
    {
        // Check boundaries
        assert((x + w) * virtualScale <= img.width());
        assert((y + h) * virtualScale <= img.height());

        m_NumVirtualRows = w;
        const uint32_t pixelSizeByte = m_Image.getChannels();
        const uint32_t virtualPixelSizeByteX = virtualPxW * pixelSizeByte;
        const uint32_t xOffsetByteSize = virtualOffsetX * virtualPixelSizeByteX;
        uint32_t virtualRowByteSize = (virtualPxW * pixelSizeByte);


        m_RawBuffer = m_Image.data();
        m_VirtualRowSizeByte = m_Image.getChannels() * m_Image.getWidth() * virtualScale;
    }

    anubis::MemoryView<T> operator[](size_t rowIndex) 
    {
        const uint32_t rowByteOffset


        {
            size_t pixelStride = m_Image.getChannels(); // We assume one byte per channel. Stride is the length of one pixel (RGB..) in bytes
            size_t virtualPixelStride = pixelStride * m_VirtualPixelWidth;
            size_t bufferOffset = m_RowSizeByte * m_VirtualPixelHeight * rowIndex;

            RawBuffer_t rowBuffer = m_RawBuffer + bufferOffset;
            return anubis::MemoryView<T>(rowBuffer, m_RowSizeByte, 0, virtualPixelStride);
        }
    }

    Iterator begin(void) noexcept { return Iterator(*this, 0); }
    Iterator end(void) noexcept { return Iterator(*this, size()); }

    // Return the number of virtual rows of the picture
    size_t size(void) const noexcept {
        return m_NumVirtualRows;
    }


private:

    //uint32_t m_Scale;
    Image_t& m_Image;
    RawBuffer_t m_RawBuffer;

    uint32_t m_NumVirtualRows; 
    // Virtual size is how much of pixel is a "virtual" pixel composed of
    // For example if we want to process an image per pixel, but the image was scaled up, we can use virtual pixels
    uint32_t m_VirtualPixelWidth, m_VirtualPixelHeight;
    uint32_t m_VirtualRowSizeByte;
};


class Image
{
public:
    //Image(void);
    Image(const std::string& filePath, uint32_t numChannels = 0)
        : m_Data(nullptr, stbi_image_free), m_NumChannels(0), m_Width(0), m_Height(0)
    {

        int w, h, comp;
        stbi_uc* pixels = stbi_load("data/invaders_ref.png", &w, &h, &comp, numChannels);

        if (!pixels) {
            throw std::runtime_error("Failed to loag image \"" + filePath + "\"");
        }
        
        m_Data.reset(pixels);
        m_NumChannels = numChannels ? numChannels : comp;
        m_Width = w;
        m_Height = h;
    }

    template <typename T>
    ImageView<T> getView(uint32_t virtualPixelOffsetX, uint32_t virtualPixelOffsetY, uint32_t virtualWidth, uint32_t virtualHeight, size_t virtualPixelSize = 1) {
        re
    }


    uint32_t getWidth(void) const noexcept { return m_Width; }
    uint32_t getHeight(void) const noexcept { return m_Height; }
    uint32_t getChannels(void) const noexcept { return m_NumChannels; }
    
    const uint8_t* data(void) const noexcept { return m_Data.get(); }
    uint8_t* data(void) noexcept { return m_Data.get(); }
private:
    std::unique_ptr<uint8_t, void(*)(void*)> m_Data;
    uint32_t m_NumChannels;
    uint32_t m_Width, m_Height;
};



//std::map<char, 

void parseSpaceInvaders(void) {

    Image refInvaders("data/invaders_ref.png", 3);

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




    std::cout << "Pixel size=" << pixelScale << ", " << tileWidth << "x" << tileHeight << std::endl;

}

int main(void)
{
    parseSpaceInvaders();
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
        std::string_view base64Data(data.begin() + endLine + 1, data.end()); // remove the -- Anubis -- header sent by the server
        std::cout << "base64 data length=" << base64Data.length() << std::endl;
        std::vector<uint8_t> pixels = base64Decode(base64Data);
        std::string_view pixelView((const char*)pixels.data(), pixels.size());
        //std::cout << pixelView << std::endl;
        //std::cout << pixels.size() << std::endl;
        std::ofstream pngout("data/out.png", std::ios::binary);
        pngout.write((const char*)pixels.data(), pixels.size());

       // stbi_write_bmp("Out.bmp", 40, 20, 3, pixels.data());
    }
    else {
        std::cout << "error while reading : " << result << std::endl;
    }

    return EXIT_SUCCESS;
}