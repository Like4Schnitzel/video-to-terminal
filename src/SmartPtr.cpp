#include "SmartPtr.hpp"

template <typename T>
SmartPtr<T>::SmartPtr()
{
    size = 0;
    ptr = (T*) malloc(0);
}

template <typename T>
SmartPtr<T>::SmartPtr(const int s)
{
    size = s;
    ptr = (T*)malloc(s * sizeof(T));
}

template <typename T>
SmartPtr<T>::~SmartPtr()
{
    free(ptr);
}

template <typename T>
void SmartPtr<T>::resize(const int s)
{
    assert(s >= 0);
    
    ptr = (T*) realloc(ptr, s*sizeof(T));
}

template <typename T>
void SmartPtr<T>::set(const T val, const int position)
{
    assert(position >= 0 && position < size);

    ptr[position] = val;
}

template <typename T>
T SmartPtr<T>::get(const int position)
{
    assert(position >= 0 && position < size);

    return ptr[position];
}
