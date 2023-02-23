#ifndef ANUBIS_MEMORY_VIEW_HPP
#define ANUBIS_MEMORY_VIEW_HPP

#include <type_traits>
#include <anubis/util/Iterator.hpp>

namespace anubis
{

template <typename T>
class MemoryView
{
    template <bool IsConst>
    class Iterator_t;


public:
    using RawBuffer_t = std::conditional_t<std::is_const_v<T>, const void*, void*>;
    using ValueType = T;
    using ReferenceType = T&;
    using Iterator = GenericIterator<MemoryView, T&>;
    using ConstIterator = GenericIterator<const MemoryView, const T&>;

public:

    MemoryView(RawBuffer_t pBuffer, size_t bufferByteSize, size_t offset = 0, size_t stride = sizeof(T));

    Iterator begin(void) noexcept;
    ConstIterator begin(void) const noexcept;
    Iterator end(void) noexcept;
    ConstIterator end(void) const noexcept;

    size_t size(void) const noexcept;
    const T& operator[](size_t index) const;
    T& operator[](size_t index);

private:
    T& getValueByIndex(size_t index) const;

    RawBuffer_t m_RawBuffer;
    size_t m_RawBufferSizeBytes;
    size_t m_VirtualElementStartOffset; // Required: 
    size_t m_VirtualElementStride; // Required : m_RawBufferSizeBytes % stride == 0
    static constexpr size_t m_ElementSize = sizeof(T);

};

}

#include "anubis/util/inl/MemoryView.inl"

#endif // !ANUBIS_MEMORY_VIEW_HPP