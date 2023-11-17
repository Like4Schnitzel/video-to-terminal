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
    this->bytes = SmartPtr<Byte>(buf);
    this->bits = SmartPtr<bool>(8*buf);
    this->index = 0;

    readFileBytesToBuffer(buf);
}

void BitStream::readFileBytesToBuffer(int n)
{
    const int bitsToReplace = n*8;
    // shift elements to the left
    for (int i = 0; i < bufferSize - n; i++)
    {
        bytes.set(bytes.get(i+n), i);
    }
    for (int i = 0; i < bitBufferSize - bitsToReplace; i++)
    {
        bits.set(bits.get(i+bitsToReplace), i);
    }

    // fill elements on the right up with read content
    SmartPtr<char> readBytes = SmartPtr<char>(n);
    (*inFile).read(readBytes.unsafeData(), n);
    for (int i = 0; i < n; i++)
    {
        const int bytesIndex = bufferSize-n+i;
        const int eightTimesBytesIndex = 8*bytesIndex;
        bytes.set(readBytes.get(i), bytesIndex);

        for (int j = 0; j < 8; j++)
        {
            bits.set((bytes.get(bytesIndex) >> (7-j)) & 0b1, eightTimesBytesIndex+j);
        }
    }
}

SmartPtr<bool> BitStream::readBits(int n)
{
    SmartPtr<bool> result = SmartPtr<bool>(n);

    for (int i = 0; i < n; i++)
    {
        result.set(bits.get(index), i);
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
