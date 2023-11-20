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
        static auto bitArrayToByteArray(const bool* bits, const ulong bitlen);

        template <typename T>
        static void pushArray(std::vector<T>* vec, const T* arr, const ulong arrLen)
        {
            for (ulong i = 0; i < arrLen; i++) {
                (*vec).push_back(arr[i]);
            }
        }

        static void writeToFile(const std::string fileName, const char* bytes, const ulong byteSize, const bool append);

        static auto numToByteArray(const float num);

        template <typename uints>
        static auto numToByteArray(uints num)
        {
            const int byteSize = sizeof(num);

            std::array<Byte, sizeof(uints)> arr;
            for (int i = 0; i < byteSize; i++)
            {
                arr[i] = num >> ((byteSize-i-1) * 8) & 0xFF;
            }
            return arr;
        }

        static ulong byteArrayToUint(const Byte* arr, const int arrLen);

        static float byteArrayToFloat(const Byte* arr, const int arrLen);

        static auto charInfoToByteArray(const CharInfo ci);

        static auto byteArrayToBitArray(const Byte* input, int inputLen);
};
