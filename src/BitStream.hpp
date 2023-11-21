#pragma once

#include "BinaryUtils.hpp"

#include <fstream>

class BitStream{
    private:
        std::ifstream* inFile;
        std::vector<Byte> bytes;
        std::vector<bool> bits;
        int index;
        int bufferSize;
        int bitBufferSize;

        void readFileBytesToBuffer(int n);
    public:
        BitStream();
        BitStream(std::ifstream* inFile, int bufferSize);
        std::vector<Byte> readBytes(int n);
        std::vector<bool> readBits(int n);

        int getIndex();
};
