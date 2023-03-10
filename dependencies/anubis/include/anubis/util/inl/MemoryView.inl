#ifndef ANUBIS_MEMORY_VIEW
#include "anubis/util/MemoryView.hpp"
#endif // !ANUBIS_MEMORY_VIEW_HPP

#include <stdexcept>

namespace anubis
{

//-----------------------------------------------
template <typename T>
MemoryView<T>::MemoryView(RawBuffer_t pBuffer, size_t bufferByteSize, size_t offset, size_t stride)
    : m_RawBuffer(pBuffer), m_RawBufferSizeBytes(bufferByteSize), m_VirtualElementStartOffset(offset), m_VirtualElementStride(stride)
{

}

//-----------------------------------------------
template <typename T>
typename MemoryView<T>::Iterator MemoryView<T>::begin(void) noexcept
{
    return Iterator(*this, 0);
}

//-----------------------------------------------
template <typename T>
typename MemoryView<T>::ConstIterator MemoryView<T>::begin(void) const noexcept
{
    return Iterator(*this, 0);
}

//-----------------------------------------------
template <typename T>
typename MemoryView<T>::Iterator MemoryView<T>::end(void) noexcept
{
    return Iterator(*this, size());
}

//-----------------------------------------------
template <typename T>
typename MemoryView<T>::ConstIterator MemoryView<T>::end(void) const noexcept
{
    return Iterator(*this, size());
}

//-----------------------------------------------
template <typename T>
size_t MemoryView<T>::size(void) const noexcept 
{
    const size_t removedElements = (m_VirtualElementStartOffset + m_ElementSize - 1) / m_VirtualElementStride;
    const size_t numElements = (m_RawBufferSizeBytes / m_VirtualElementStride);
    return (numElements - removedElements);
}

//-----------------------------------------------
template <typename T>
const T& MemoryView<T>::operator[](size_t index) const 
{
    return getValueByIndex(index);
}

//-----------------------------------------------
template <typename T>
T& MemoryView<T>::operator[](size_t index) 
{
    return getValueByIndex(index);
}

//-----------------------------------------------
template <typename T>
T& MemoryView<T>::getValueByIndex(size_t index) const 
{
    size_t byteOffset = m_VirtualElementStartOffset + (m_VirtualElementStride * index);
    size_t endOffset = byteOffset + m_ElementSize;

    if (endOffset > m_RawBufferSizeBytes) {
        throw std::runtime_error("MemoryViex: index out of bounds");
    }

    using ByteBuffer_t = std::conditional_t<std::is_const_v<T>, const uint8_t*, uint8_t*>;

    ByteBuffer_t dataAddress = reinterpret_cast<ByteBuffer_t>(m_RawBuffer) + byteOffset;
    return *reinterpret_cast<T*>(dataAddress);
}

}