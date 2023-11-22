#pragma once

#include "BinaryUtils.hpp"

#include <fstream>

class BitStream{
    private:
        std::ifstream* inFile;
        std::vector<Byte> bytes;
        int index;
        int bufferSize;

        void readFileBytesToBuffer(int n);
    public:
        BitStream();
        BitStream(std::ifstream* inFile, int bufferSize);
        std::vector<Byte> readBytes(int n);
        std::vector<bool> readBits(int n);

        int getIndex();
};
