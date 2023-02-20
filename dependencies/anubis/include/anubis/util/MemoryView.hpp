#ifndef ANUBIS_MEMORY_VIEW_HPP
#define ANUBIS_MEMORY_VIEW_HPP

#include <type_traits>

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
    using Iterator = Iterator_t<false>;
    using ConstIterator = Iterator_t<true>;

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

private:

    template <bool IsConst>
    class Iterator_t
    {
    public:
        using MemoryView_t = std::conditional_t<IsConst, const MemoryView, MemoryView>;

        Iterator_t(MemoryView_t& mv, size_t cursor);

        bool operator==(const Iterator_t&) const noexcept = default;
        bool operator!=(const Iterator_t&) const noexcept = default;
        Iterator_t& operator++(void) noexcept;
        Iterator_t operator++(int) noexcept;

        Iterator_t& operator--(void) noexcept;
        Iterator_t operator--(int) noexcept;

        Iterator_t operator+(size_t val) const noexcept;
        Iterator_t operator-(size_t val) const noexcept;

        Iterator_t& operator+=(size_t val) noexcept;
        Iterator_t& operator-=(size_t val) noexcept;

        MemoryView_t::ReferenceType operator*(void) const noexcept;

    private:
        MemoryView_t* m_MemoryView;
        size_t m_Cursor;
    };
};
}

#include "anubis/util/inl/MemoryView.inl"
#include "anubis/util/inl/MemoryViewIterator.inl"

#endif // !ANUBIS_MEMORY_VIEW_HPP