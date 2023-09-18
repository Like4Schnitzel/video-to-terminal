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

struct CharArrayWithSize {
    char* arr;
    ulong size;

    ~CharArrayWithSize()
    {
        free(arr);
    }
};

class BinaryUtils {
    public:
        template <typename T>
        static char* bitArrayToCharArray(T bits, ulong len)
        {
            if (len % 8 != 0)
            {
                throw std::logic_error("Bits are not divisible by 8.");
            }

            ulong byteSize = len / CHAR_BIT;
            char* output = (char*)malloc(byteSize);
            for (ulong i = 0; i < byteSize; i++)
            {
                output[i] = 0;
                // write bits to char
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

        static void writeToFile(const std::string, const std::vector<char>);

        template <typename uints>
        static bool* numToBitArray(uints num)
        {
            const int byteSize = sizeof(num);
            //std::cout << "byteSize is " << byteSize << "\n";

            bool* arr = (bool*)malloc(byteSize*CHAR_BIT);
            for (int i = byteSize*CHAR_BIT-1; i >= 0; i--)
            {
                arr[i] = num % 2;
                num /= 2;
            }
            return arr;
        }

        static bool* numToBitArray(const float);

        template <typename uints>
        static char* numToCharArray(uints num)
        {
            const int byteSize = sizeof(num);

            char* arr = (char*)malloc(byteSize);
            for (int i = byteSize-1; i >= 0; i--)
            {
                arr[i] = num % 256;
                num /= 256;
            }
            return arr;
        }

        static char* numToCharArray(const float);

        static bool* charInfoToBitArray(const CharInfo ci);

        static char* charInfoToCharArray(const CharInfo ci);

        static CharArrayWithSize compressBytes(const char* input, const ulong inputLength);
};
