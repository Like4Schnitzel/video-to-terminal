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
#include "CharInfoStruct.hpp"
#include "SmartPtr.hpp"

class BinaryUtils {
    public:
        static SmartPtr<Byte> bitArrayToByteArray(const SmartPtr<bool> bits);

        template <typename T>
        static void pushArray(std::vector<T>* vec, const SmartPtr<T> arr)
        {
            for (ulong i = 0; i < arr.size; i++) {
                (*vec).push_back(arr.get(i));
                //std::cout << arr[i];
            }
            //std::cout << " pushed to vector.\nVector size is " << (*vec).size() << "\n";
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
