#pragma once

#include "BinaryUtils.hpp"
#include <fstream>

class BitStream{
    private:
        std::ifstream* inFile;
        char* bytes;
        bool* bits;
        int index;
        int bufferSize;

        void readFileBytesToBuffer(int n);
    public:
        BitStream();
        BitStream(std::ifstream* inFile, int bufferSize);
        ~BitStream();
        bool* readBits(int n);
};
