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
    this->bytes.resize(buf);
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

    // fill elements on the right up with read content
    std::unique_ptr<char[]> readBytes = std::make_unique<char[]>(n);
    (*inFile).read(readBytes.get(), n);
    for (int i = 0; i < n; i++)
    {
        const int bytesIndex = bufferSize-n+i;
        const int eightTimesBytesIndex = 8*bytesIndex;
        bytes[bytesIndex] = readBytes[i];
    }
}

std::vector<Byte> BitStream::readBytes(int n)
{
    std::vector<Byte> result;
    result.reserve(n);

    const int endIndex = index-1;
    for (int i = 0; i < n; i++)
    {
        Byte b = 0;
        Byte bits = bytes[i];
        int bitsToProcess = 7;
        for (int j = index; j < 8; j++)
        {
            b |= (bits >> (7-j) & 1) << bitsToProcess;
            bitsToProcess--;
        }

        bits = bytes[i+1];
        for (int j = 0; j < index; j++)
        {
            b |= (bits >> (7-j) & 1) << bitsToProcess;
            bitsToProcess--;
        }
        result.push_back(b);
    }

    readFileBytesToBuffer(n);
    return result;
}

std::vector<bool> BitStream::readBits(int n)
{
    std::vector<bool> result;
    result.reserve(n);

    const int endIndex = index + n;

    if (endIndex < 8)
    {
        Byte currentlyProcessing = bytes[0];
        for (int j = index; j < endIndex; j++)
        {
            result.push_back(currentlyProcessing >> (7-j) & 1);
        }
        index = endIndex;
    }
    else
    {
        const int byteRange = endIndex / 8;
        const int trailingBits = endIndex % 8;

        int bi = 0; // byteIndex
        // first byte
        Byte currentlyProcessing = bytes[bi];
        for (int j = index; j < 8; j++)
        {
            result.push_back(currentlyProcessing >> (7-j) & 1);
        }
        bi++;

        // inbetween full bytes
        for (; bi < byteRange; bi++)
        {
            currentlyProcessing = bytes[bi];
            for (int j = 0; j < 8; j++)
            {
                result.push_back(currentlyProcessing >> (7-j) & 1);
            }
        }

        // last byte
        currentlyProcessing = bytes[bi];
        for (int j = 0; j < trailingBits; j++)
        {
            result.push_back(currentlyProcessing >> (7-j) & 1);
        }

        index = endIndex;
        if (index > 7)
        {
            readFileBytesToBuffer(index / 8);
            index %= 8;
        }
    }

    return result;
}

int BitStream::getIndex()
{
    return this->index;
}
