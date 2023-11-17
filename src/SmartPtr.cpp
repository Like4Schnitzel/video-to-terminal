#include "SmartPtr.hpp"

template <typename T>
SmartPtr<T>::SmartPtr()
{
    size = 0;
    ptr = (T*)malloc(0);
}

template <typename T>
SmartPtr<T>::SmartPtr(const int s)
{
    size = s;
    ptr = (T*)malloc(s * sizeof(T));
}

template <typename T>
SmartPtr<T>::SmartPtr(const T* arr, int s)
{
    ptr = arr;
    size = s;
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
void SmartPtr<T>::set(const int position, const T val)
{
    assert(position >= 0 && position < size);

    ptr[position] = val;
}

template <typename T>
const T SmartPtr<T>::get(const int position)
{
    assert(position >= 0 && position < size);

    return ptr[position];
}

template <typename T>
const int SmartPtr<T>::getSize()
{
    return size;
}

template <typename T>
const T* SmartPtr<T>::data()
{
    return ptr;
}

template <typename T>
T* SmartPtr<T>::unsafeData()
{
    return ptr;
}
