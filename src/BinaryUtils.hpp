#pragma once

#include <vector>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <bitset>
#include <climits>
#include "VideoTranscoder.hpp"

class BinaryUtils {
    public:
        static void pushArray(std::vector<bool>*, bool*, int);
        static void writeToFile(std::string, std::vector<bool>);

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

        static bool* numToBitArray(float);

        static bool* charInfoToBitArray(CharInfo ci);
};
