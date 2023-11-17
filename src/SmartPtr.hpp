#pragma once

#include <assert.h>

template <typename T>
class SmartPtr {
    private:
        T* ptr;
        int size;
    public:
        SmartPtr();
        SmartPtr(const int s);
        ~SmartPtr();

        void resize(const int s);
        void set(const T val, const int position);
        T get(const int position);
        const T* data();
};