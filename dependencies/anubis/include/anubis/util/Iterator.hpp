#ifndef ANUBIS_ITERATOR_HPP
#define ANUBIS_ITERATOR_HPP

#include <type_traits>

namespace anubis
{

    template <typename T_SRC, typename T = T_SRC&>
    class GenericIterator
    {
    public:
        GenericIterator(T_SRC& dataSource, size_t cursor);

        bool operator==(const GenericIterator&) const noexcept = default;
        bool operator!=(const GenericIterator&) const noexcept = default;
        GenericIterator& operator++(void) noexcept;
        GenericIterator operator++(int) noexcept;

        GenericIterator& operator--(void) noexcept;
        GenericIterator operator--(int) noexcept;

        GenericIterator operator+(size_t val) const noexcept;
        GenericIterator operator-(size_t val) const noexcept;

        GenericIterator& operator+=(size_t val) noexcept;
        GenericIterator& operator-=(size_t val) noexcept;

        T operator*(void) const noexcept;
    private:
        T_SRC* m_DataSource;
        size_t m_Cursor;
    };
}

#include "inl/Iterator.inl"

#endif // !ANUBIS_ITERATOR_HPP
