#ifndef ANUBIS_ITERATOR_HPP
#include "anubis/util/Iterator.hpp"
#endif // !ANUBIS_ITERATOR_HPP


namespace anubis
{

    //-----------------------------------------------
    template <typename T_SRC, typename T>
    GenericIterator<T_SRC, T>::GenericIterator(T_SRC& dataSource, size_t cursor)
        : m_DataSource(&dataSource), m_Cursor(cursor)
    {

    }

    //-----------------------------------------------
    template <typename T_SRC, typename T>
    GenericIterator<T_SRC, T>& GenericIterator<T_SRC, T>::operator++(void) noexcept
    {
        m_Cursor++;
        return *this;
    }

    //-----------------------------------------------
    template <typename T_SRC, typename T>
    GenericIterator<T_SRC, T> GenericIterator<T_SRC, T>::operator++(int) noexcept
    {
        return GenericIterator(*this);
        m_Cursor++;
    }

    //-----------------------------------------------
    template <typename T_SRC, typename T>
    GenericIterator<T_SRC, T>& GenericIterator<T_SRC, T>::operator--(void) noexcept
    {
        m_Cursor--;
        return *this;
    }

    //-----------------------------------------------
    template <typename T_SRC, typename T>
    GenericIterator<T_SRC, T> GenericIterator<T_SRC, T>::operator--(int) noexcept
    {
        return GenericIterator(*this);
        m_Cursor--;
    }

    //----------------------------------------------- 
    template <typename T_SRC, typename T>
    GenericIterator<T_SRC, T> GenericIterator<T_SRC, T>::operator+(size_t val) const noexcept
    {
        return GenericIterator(*m_DataSource, m_Cursor + 1);
    }

    //----------------------------------------------- 
    template <typename T_SRC, typename T>
    GenericIterator<T_SRC, T> GenericIterator<T_SRC, T>::operator-(size_t val) const noexcept
    {
        return GenericIterator(*m_DataSource, m_Cursor - 1);
    }

    //----------------------------------------------- 
    template <typename T_SRC, typename T>
    GenericIterator<T_SRC, T>& GenericIterator<T_SRC, T>::operator+=(size_t val) noexcept
    {
        m_Cursor += val;
        return *this;
    }

    //----------------------------------------------- 
    template <typename T_SRC, typename T>
    GenericIterator<T_SRC, T>& GenericIterator<T_SRC, T>::operator-=(size_t val) noexcept {
        m_Cursor -= val;
        return *this;
    }

    //----------------------------------------------- 
    template<typename T_SRC, typename T>
    T GenericIterator<T_SRC, T>::operator*(void) const noexcept
    {
        return (*m_DataSource)[m_Cursor];
    }
}