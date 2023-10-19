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
#include "VideoTranscoder.hpp"

class BinaryUtils {
    public:
        template <typename T>
        static Byte* bitArrayToByteArray(T bits, ulong len)
        {
            if (len % 8 != 0)
            {
                throw std::logic_error("Bits are not divisible by 8.");
            }

            ulong byteSize = len / CHAR_BIT;
            Byte* output = (Byte*)malloc(byteSize);
            for (ulong i = 0; i < byteSize; i++)
            {
                output[i] = 0;
                // write bits to byte
                for (int j = 0; j < 8; j++)
                {
                    output[i] = output[i] << 1 | bits[i*8 + j];   //push already written bits to the left by one, then write 0 or 1 on the very right
                }
            }

            return output;
        }

        template <typename T>
        static void pushArray(std::vector<T>* vec, const T* arr, const ulong arrLen)
        {
            for (ulong i = 0; i < arrLen; i++) {
                (*vec).push_back(arr[i]);
                //std::cout << arr[i];
            }
            //std::cout << " pushed to vector.\nVector size is " << (*vec).size() << "\n";
        }

        static void writeToFile(const std::string fileName, const std::vector<char> bytes, const bool append);

        static void writeToFile(const std::string fileName, char* bytes, const ulong byteSize, const bool append, const bool freeArr = false);

        static Byte* numToByteArray(const float);

        template <typename uints>
        static Byte* numToByteArray(uints num)
        {
            const int byteSize = sizeof(num);

            Byte* arr = (Byte*)malloc(byteSize);
            for (int i = byteSize-1; i >= 0; i--)
            {
                arr[i] = num % 256;
                num /= 256;
            }
            return arr;
        }

        static ulong byteArrayToUint(const Byte* arr, const int len);

        static float byteArrayToFloat(const Byte* arr, const int len);

        static Byte* charInfoToByteArray(const CharInfo ci);

        static bool* byteArrayToBitArray(const Byte* input, const int inputSize);
};
