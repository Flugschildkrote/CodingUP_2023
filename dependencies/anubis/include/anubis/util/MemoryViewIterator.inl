#ifndef ANUBIS_MEMORY_VIEW
#include "anubis/util/MemoryView.hpp"
#endif // !ANUBIS_MEMORY_VIEW_HPP

namespace anubis
{

//-----------------------------------------------
template <typename T>
template <bool IsConst>
MemoryView<T>::Iterator_t<IsConst>::Iterator_t(MemoryView_t& mv, size_t cursor)
    : m_MemoryView(&mv), m_Cursor(cursor)
{

}

//-----------------------------------------------
template <typename T>
template <bool IsConst>
MemoryView<T>::Iterator_t<IsConst>& MemoryView<T>::Iterator_t<IsConst>::operator++(void) noexcept
{
    m_Cursor++;
    return *this;
}

//-----------------------------------------------
template <typename T>
template <bool IsConst>
MemoryView<T>::Iterator_t<IsConst> MemoryView<T>::Iterator_t<IsConst>::operator++(int) noexcept
{
    return Iterator_t(*this);
    m_Cursor++;
}

//-----------------------------------------------
template <typename T>
template <bool IsConst>
MemoryView<T>::Iterator_t<IsConst>& MemoryView<T>::Iterator_t<IsConst>::operator--(void) noexcept
{
    m_Cursor--;
    return *this;
}

//-----------------------------------------------
template <typename T>
template <bool IsConst>
MemoryView<T>::Iterator_t<IsConst> MemoryView<T>::Iterator_t<IsConst>::operator--(int) noexcept
{
    return Iterator_t(*this);
    m_Cursor--;
}

//----------------------------------------------- 
template <typename T>
template <bool IsConst>
MemoryView<T>::Iterator_t<IsConst> MemoryView<T>::Iterator_t<IsConst>::operator+(size_t val) const noexcept
{
    return Iterator_t(*m_MemoryView, m_Cursor + 1);
}

//----------------------------------------------- 
template <typename T>
template <bool IsConst>
MemoryView<T>::Iterator_t<IsConst> MemoryView<T>::Iterator_t<IsConst>::operator-(size_t val) const noexcept
{
    return Iterator_t(*m_MemoryView, m_Cursor - 1);
}

//----------------------------------------------- 
template <typename T>
template <bool IsConst>
MemoryView<T>::Iterator_t<IsConst>& MemoryView<T>::Iterator_t<IsConst>::operator+=(size_t val) noexcept
{
    m_Cursor += val;
    return *this;
}

//----------------------------------------------- 
template <typename T>
template <bool IsConst>
MemoryView<T>::Iterator_t<IsConst>& MemoryView<T>::Iterator_t<IsConst>::operator-=(size_t val) noexcept {
    m_Cursor -= val;
    return *this;
}

//----------------------------------------------- 
template<typename T>
template<bool IsConst>
MemoryView<T>::Iterator_t<IsConst>::MemoryView_t::ReferenceType MemoryView<T>::Iterator_t<IsConst>::operator*(void) const noexcept
{
    return (*m_MemoryView)[m_Cursor];
}

}