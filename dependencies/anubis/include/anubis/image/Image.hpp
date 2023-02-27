#ifndef ANUBIS_IMAGE_HPP
#define ANUBIS_IMAGE_HPP

#include "anubis/image/ImageView.hpp"
#include <memory>
#include <string>

namespace anubis
{
    class Image
    {
    public:
        Image(void);

        Image(Image&&) = default;
        Image& operator=(Image&&) = default;

        Image(const std::string& filePath, uint32_t numChannels = 0);

        /**
         * @brief Load an image from memory. The buffer must contain a real image (png, jpeg, etc..)
         * @param bufferSize Number of element in pBuffer
         * 
         */
        template <typename T>
        Image(const T* pBuffer, size_t bufferSize, uint32_t numChannels);

        template <typename T>
        ImageView<T> getView(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t virtualPixelSize = 1);

        uint32_t getWidth(void) const noexcept;
        uint32_t getHeight(void) const noexcept;
        uint32_t getChannels(void) const noexcept;

        const uint8_t* data(void) const noexcept;
        uint8_t* data(void) noexcept;
    private:
        std::unique_ptr<uint8_t, void(*)(void*)> m_Data;
        uint32_t m_NumChannels;
        uint32_t m_Width, m_Height;
    };

    template <typename T>
    Image::Image(const T* pBuffer, size_t bufferSize, uint32_t numChannels)
        : m_Data(nullptr, stbi_image_free), m_NumChannels(0), m_Width(0), m_Height(0)
    {
        int w, h, comp;
        int sizeByte = int(sizeof(T) * bufferSize);
        stbi_uc* pixels = stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(pBuffer), sizeByte, &w, &h, &comp, m_NumChannels);
        if (!pixels) {
            std::cout << stbi_failure_reason() << std::endl;
            throw std::runtime_error("Failed to loag image from buffer");
        }

        m_Data.reset(pixels);
        m_NumChannels = numChannels ? numChannels : comp;
        m_Width = w;
        m_Height = h;
    }

    template <typename T>
    ImageView<T> getView(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t virtualPixelSize = 1) {
        return ImageView<T>(*this, x, y, w, h, virtualPixelSize);
    }


}

#endif // !ANUBIS_IMAGE_HPP
