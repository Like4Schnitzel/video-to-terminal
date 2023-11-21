#include "BitStream.hpp"

BitStream::BitStream()
{
    this->inFile = nullptr;
    this->bufferSize = 0;
    this->index = 0;
}

BitStream::BitStream(std::ifstream* inF, int buf)
{
    this->inFile = inF;
    this->bufferSize = buf;
    this->bitBufferSize = buf*8;
    this->bytes.reserve(buf);
    this->bits.reserve(8*buf);
    this->index = 0;

    readFileBytesToBuffer(buf);
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
    std::vector<char> readBytes;
    readBytes.reserve(n);
    (*inFile).read(readBytes.data(), n);
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
}

std::vector<Byte> BitStream::readBytes(int n)
{
    std::vector<Byte> result;
    result.resize(n, 0);

    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            result[i] = result[i] | (bits[index] << (7 - j));
            index++;
        }
    }

    readFileBytesToBuffer(n);
    index -= 8*n;
    return result;
}

std::vector<bool> BitStream::readBits(int n)
{
    std::vector<bool> result;
    result.reserve(n);

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
