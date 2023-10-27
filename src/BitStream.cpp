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
    this->bitBufferSize = buf*8;
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
    const int bitsToReplace = n*8;
    // shift elements to the left
    for (int i = 0; i < bufferSize - n; i++)
    {
        bytes[i] = bytes[i+n];
    }
    for (int i = 0; i < bitBufferSize - bitsToReplace; i++)
    {
        bits[i] = bits[i+bitsToReplace];
    }

    // fill elements on the right up with read content
    char* readBytes = (char*)malloc(n*sizeof(char));
    (*inFile).read(readBytes, n);
    for (int i = 0; i < n; i++)
    {
        const int bytesIndex = bufferSize-n+i;
        const int eightTimesBytesIndex = 8*bytesIndex;
        bytes[bytesIndex] = readBytes[i];

        for (int j = 0; j < 8; j++)
        {
            bits[eightTimesBytesIndex+j] = (bytes[bytesIndex] >> (7-j)) & 0b1;
        }
    }
    free(readBytes);
}

bool* BitStream::readBits(int n)
{
    bool* result = (bool*)malloc(n*sizeof(bool));

    for (int i = 0; i < n; i++)
    {
        result[i] = bits[index];
        index++;
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
