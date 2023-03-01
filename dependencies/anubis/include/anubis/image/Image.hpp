#ifndef ANUBIS_IMAGE_HPP
#define ANUBIS_IMAGE_HPP

#include "anubis/image/ImageView.hpp"
#include <type_traits>
#include <memory>
#include <string>

namespace anubis
{
    template <typename T>
    using IsNotVoid = std::enable_if_t<std::negation_v<std::is_void<T>>>;

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
        template <typename T, typename = IsNotVoid<T>>
        Image(const T* pBuffer, size_t bufferSize, uint32_t numChannels);

        template <typename T>
        ImageView<T> getView(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t virtualPixelSize = 1);

        uint32_t getWidth(void) const noexcept;
        uint32_t getHeight(void) const noexcept;
        uint32_t getChannels(void) const noexcept;

        const uint8_t* data(void) const noexcept;
        uint8_t* data(void) noexcept;
    private:
        Image(const void* pFileData, size_t byteSize, uint32_t desiredChannelCount);

        std::unique_ptr<uint8_t, void(*)(void*)> m_Data;
        uint32_t m_NumChannels;
        uint32_t m_Width, m_Height;
    };

    template <typename T, typename>
    Image::Image(const T* pBuffer, size_t bufferSize, uint32_t numChannels)
        : Image(reinterpret_cast<const void*>(pBuffer), bufferSize*sizeof(T), numChannels)
    {

    }

    template <typename T>
    ImageView<T> Image::getView(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t virtualPixelSize)
    {
        return ImageView<T>(*this, x, y, w, h, virtualPixelSize);
    }


}

#endif // !ANUBIS_IMAGE_HPP
