#include "BitStream.hpp"

BitStream::BitStream()
{
    this->inFile = nullptr;
    this->bufferSize = 0;
    this->bytes = nullptr;
    this->bits = nullptr;
    this->index = 0;
}

BitStream::BitStream(std::ifstream* inF, int buf)
{
    this->inFile = inF;
    this->bufferSize = buf;
    this->bytes = (Byte*)malloc(buf*sizeof(Byte));
    this->bits = (bool*)malloc(8*buf*sizeof(bool));
    this->index = 0;

    readFileBytesToBuffer(buf);
}

BitStream::~BitStream()
{
    free(this->bytes);
    free(this->bits);
}

void BitStream::readFileBytesToBuffer(int n)
{
    for (int i = 0; i < n - bufferSize; i++)
    {
        bytes[i] = bytes[i+n];
    }

    char* readBytes = (char*)malloc(n*sizeof(char));
    (*inFile).read(readBytes, n);
    for (int i = 0; i < n; i++)
    {
        bytes[bufferSize-n+i] = readBytes[i];
    }
    free(readBytes);

    free(bits);
    bits = BinaryUtils::byteArrayToBitArray(bytes, bufferSize);
}

bool* BitStream::readBits(int n)
{
    bool* result = (bool*)malloc(n*sizeof(bool));

    for (int i = 0; i < n; i++)
    {
        result[i] = bits[index++];
    }

    if (index > 7)
    {
        readFileBytesToBuffer(index / 8);
        index %= 8;
    }

    return result;
}

int BitStream::getIndex()
{
    return this->index;
}
