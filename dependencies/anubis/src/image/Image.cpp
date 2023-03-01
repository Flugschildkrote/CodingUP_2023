#include "anubis/image/Image.hpp"
#include <stb_image.h>
#include <stdexcept>

namespace anubis
{
    
    Image::Image(void)
        : m_Data(nullptr, stbi_image_free), m_NumChannels(0), m_Width(0), m_Height(0)
    {

    }

    Image::Image(const std::string& filePath, uint32_t numChannels)
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

    Image::Image(const void* pFileData, size_t byteSize, uint32_t desiredChannelCount)
        : m_Data(nullptr, stbi_image_free), m_NumChannels(0), m_Width(0), m_Height(0)
    {
        int w, h, comp;
        stbi_uc* pixels = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(pFileData), int(byteSize), &w, &h, &comp, int(desiredChannelCount));
        if (!pixels) {
            throw std::runtime_error("Failed to loag image from buffer");
        }

        m_Data.reset(pixels);
        m_NumChannels = desiredChannelCount ? desiredChannelCount : comp;
        m_Width = w;
        m_Height = h;
    }

    uint32_t Image::getWidth(void) const noexcept
    { 
        return m_Width;
    }


    uint32_t Image::getHeight(void) const noexcept 
    { 
        return m_Height;
    }

    uint32_t Image::getChannels(void) const noexcept 
    { 
        return m_NumChannels;
    }

    const uint8_t* Image::data(void) const noexcept 
    { 
        return m_Data.get();
    }

    uint8_t* Image::data(void) noexcept 
    { 
        return m_Data.get();
    }

    

 
}