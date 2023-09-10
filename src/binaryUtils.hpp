#pragma once

#include <vector>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <fstream>
#include <stdexcept>

class BinaryUtils {
    public:
        static void pushArray(std::vector<bool>*, bool*, int);
        static void writeToFile(std::string, std::vector<bool>);

        template <typename uints>
        static bool* numToBitArray(uints num)
        {
            const int byteSize = sizeof(num);
            //std::cout << "byteSize is " << byteSize << "\n";

            bool* arr = (bool*)malloc(byteSize*8);
            for (int i = byteSize*8-1; i >= 0; i--)
            {
                arr[i] = num % 2;
                num /= 2;
            }
            return arr;
        }
};