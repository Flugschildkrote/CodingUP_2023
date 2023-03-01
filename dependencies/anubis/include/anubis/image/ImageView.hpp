#ifndef ANUBIS_IMAGE_VIEW_HPP
#define ANUBIS_IMAGE_VIEW_HPP

#include <type_traits>
#include <anubis/util/Iterator.hpp>
#include <anubis/util/MemoryView.hpp>
#include <anubis/image/ImageView.hpp>

namespace anubis
{
    class Image;


    template <typename T>
    class ImageView
    {
        using RawBuffer_t = std::conditional_t<std::is_const_v<T>, const void*, void*>;
        using BytePtr_t = std::conditional_t<std::is_const_v<T>, const uint8_t*, uint8_t*>;
        using Image_t = std::conditional_t<std::is_const_v<T>, const Image, Image>;

    public:

        using Iterator = anubis::GenericIterator<ImageView<T>, anubis::MemoryView<T>>;
        using ConstIterator = anubis::GenericIterator<ImageView<const T>, anubis::MemoryView<const T>>;

    public:

        ImageView(void) = default;

        /*
         * Construct an image view starting at (x,y) and size of (w, h)
         * All parameters are expressed in virtual space (a virtual pixel is composed of (virtualScale x virtualScale) real pixels
         */
        ImageView(Image_t& img, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t virtualScale)
            : m_Image(img)
        {
            // Check boundaries
            assert((x + w) * virtualScale <= img.getWidth());
            assert((y + h) * virtualScale <= img.getHeight());

            const uint32_t pixelSizeByte = m_Image.getChannels();

            m_RawBuffer = m_Image.data();
            m_NumRows_VirtualSpace = h;
            m_PixelByteSize_VirtualSpace = pixelSizeByte * virtualScale;
            m_RowByteSize_VirtualSpace = m_PixelByteSize_VirtualSpace * m_Image.getWidth();
            m_ViewRowByteSize = m_PixelByteSize_VirtualSpace * w;
            m_ViewXOffsetBytes = m_PixelByteSize_VirtualSpace * x;
            m_ViewYOffsetBytes = m_RowByteSize_VirtualSpace * y;
        }


        anubis::MemoryView<T> operator[](size_t rowIndex)
        {
            const uint32_t rowAddress = static_cast<uint32_t>(m_ViewYOffsetBytes + (rowIndex * m_RowByteSize_VirtualSpace)); // Offset in byte to the start of the row
            const uint32_t xOffset = m_ViewXOffsetBytes;
            RawBuffer_t viewRowAddress = reinterpret_cast<BytePtr_t>(m_RawBuffer) + rowAddress + xOffset;
            return anubis::MemoryView<T>(viewRowAddress, m_ViewRowByteSize, 0, m_PixelByteSize_VirtualSpace);
        }

        Iterator begin(void) noexcept { return Iterator(*this, 0); }
        Iterator end(void) noexcept { return Iterator(*this, size()); }

        // Return the number of virtual rows of the picture
        size_t size(void) const noexcept {
            return m_NumRows_VirtualSpace;
        }


    private:

        //uint32_t m_Scale;
        Image_t& m_Image;
        RawBuffer_t m_RawBuffer;

        uint32_t m_NumRows_VirtualSpace;
        uint32_t m_PixelByteSize_VirtualSpace;
        uint32_t m_RowByteSize_VirtualSpace;
        uint32_t m_ViewRowByteSize;

        uint32_t m_ViewXOffsetBytes;
        uint32_t m_ViewYOffsetBytes;
    };
}

#endif // !ANUBIS_IMAGE_VIEW_HPP