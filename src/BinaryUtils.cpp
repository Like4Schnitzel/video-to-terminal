#include "BinaryUtils.hpp"

void BinaryUtils::writeToFile(const std::string fileName, const std::vector<char> bytes, const bool append)
{
    const ulong byteSize = bytes.size()/CHAR_BIT;

    std::ofstream file;
    if (append)
    {
        file.open(fileName, std::ios_base::app | std::ios::binary);
    }
    else
    {
        file.open(fileName, std::ios::out | std::ios::binary);
    }

    if (!file.good())
    {
        throw std::runtime_error("Cannot open file to write to.");
    }
    file.write(bytes.data(), byteSize);
    file.close();
}

void BinaryUtils::writeToFile(const std::string fileName, char* bytes, const ulong byteSize, const bool append, const bool freeArr)
{
    std::ofstream file;
    if (append)
    {
        file.open(fileName, std::ios_base::app | std::ios::binary);
    }
    else
    {
        file.open(fileName, std::ios::out | std::ios::binary);
    }

    if (!file.good())
    {
        throw std::runtime_error("Cannot open file to write to.");
    }
    file.write(bytes, byteSize);
    file.close();

    if (freeArr)
    {
        free(bytes);
    }
}

Byte* BinaryUtils::numToByteArray(const float num)
{
    union
    {
        float input;
        int output;
    } data;
    data.input = num;
    return numToByteArray(data.output);
}

ulong BinaryUtils::byteArrayToUint(const Byte* arr, const int len)
{
    ulong num = 0;
    ulong markiplier = 1;

    for (int i = 0; i < len; i++)
    {
        num += (uint8_t) arr[len-i-1] * markiplier;    // going from right to left
        markiplier *= 256;
    }

    return num;
}

float BinaryUtils::byteArrayToFloat(const Byte* arr, const int len)
{
    if (len != 4)
    {
        std::logic_error("Can only convert char arrays of length 4 to float.");
    }

    union {
        unsigned int x;
        float f;
    } temp;
    temp.x = byteArrayToUint(arr, len);
    return temp.f;
}

Byte* BinaryUtils::charInfoToByteArray(const CharInfo ci)
{
    Byte* result = (Byte*)malloc(sizeof(CharInfo));
    int index = 0;

    for (int i = 0; i < 3; i++)
    {
        result[index] = ci.foregroundRGB[i];
        index++;
    }

    for (int i = 0; i < 3; i++)
    {
        result[index] = ci.backgroundRGB[i];
        index++;
    }

    result[index] = ci.chara;

    return result;
}

bool* BinaryUtils::byteArrayToBitArray(const Byte* input, const int inputSize)
{
    bool* result = (bool*)malloc(inputSize*sizeof(char));
    for (int i = 0; i < inputSize; i++)
    {
        char c = input[i];
        for (int j = 0; j < 8; j++)
        {
            result[8*i+j] = (c >> (7-j)) & 0b1;
        }
    }
    return result;
}
