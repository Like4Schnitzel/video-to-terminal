#pragma once

#include <vector>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <bitset>
#include <climits>
#include <zlib.h>
#include <memory>
#include <array>
#include "CharInfoStruct.hpp"

class BinaryUtils {
    public:
        static auto bitArrayToByteArray(const std::shared_ptr<bool> bits, const ulong bitlen);

        template <typename T>
        static void pushArray(std::vector<T>* vec, const std::shared_ptr<T> arr, const ulong arrLen)
        {
            for (ulong i = 0; i < arrLen; i++) {
                (*vec).push_back(arr.get()[i]);
            }
        }

        static void writeToFile(const std::string fileName, const char* bytes, const ulong byteSize, const bool append);

        static SmartPtr<Byte> numToByteArray(const float);

        template <typename uints>
        static SmartPtr<Byte> numToByteArray(uints num)
        {
            const int byteSize = sizeof(num);

            SmartPtr<Byte> arr = SmartPtr<Byte>(byteSize);
            for (int i = 0; i < byteSize; i++)
            {
                arr.set(i, num >> ((byteSize-i-1) * 8) & 0xFF);
            }
            return arr;
        }

        static ulong byteArrayToUint(SmartPtr<Byte> arr);

        static float byteArrayToFloat(SmartPtr<Byte> arr);

        static SmartPtr<Byte> charInfoToByteArray(const CharInfo ci);

        static SmartPtr<bool> byteArrayToBitArray(SmartPtr<Byte> input);
};
