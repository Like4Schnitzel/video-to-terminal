#pragma once

#include "BinaryUtils.hpp"
#include "SmartPtr.hpp"

#include <fstream>

class BitStream{
    private:
        std::ifstream* inFile;
        SmartPtr<Byte> bytes;
        SmartPtr<bool> bits;
        int index;
        int bufferSize;
        int bitBufferSize;

        void readFileBytesToBuffer(int n);
    public:
        BitStream();
        BitStream(std::ifstream* inFile, int bufferSize);
        SmartPtr<bool> readBits(int n);

        int getIndex();
};
