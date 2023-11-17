#pragma once

#include <assert.h>
#include <stdlib.h>

template <typename T>
class SmartPtr {
    private:
        T* ptr;
        int size;
    public:
        SmartPtr();
        SmartPtr(const int s);
        SmartPtr(const T* arr, const int s);
        ~SmartPtr();

        void resize(const int s);
        void set(const int position, const T val);
        const T get(const int position);
        const int getSize();
        const T* data();
        T* unsafeData();
};
