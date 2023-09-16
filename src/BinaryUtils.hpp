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

struct BoolArrayWithSize {
    bool* arr;
    int size;
};

class BinaryUtils {
    public:
        static void pushArray(std::vector<bool>*, const bool*, const int);
        static void writeToFile(const std::string, const std::vector<bool>);

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

        static bool* charInfoToBitArray(const CharInfo ci);

        static BoolArrayWithSize compressBits(const bool* input, const int inputLength);
};
