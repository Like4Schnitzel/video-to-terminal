#include "BinaryUtils.hpp"

auto BinaryUtils::bitArrayToByteArray(std::shared_ptr<bool> bits, const ulong bitLen)
{
    if (bitLen % 8 != 0)
    {
        throw std::logic_error("Bits are not divisible by 8.");
    }

    const ulong byteSize = bitLen / CHAR_BIT;
    std::vector<Byte> output;
    output.resize(byteSize, 0);

    for (ulong i = 0; i < byteSize; i++)
    {
        // write bits to byte
        for (int j = 0; j < 8; j++)
        {
            // push already written bits to the left by one, then write 0 or 1 on the very right
            output[i] = output[i] << 1 | bits.get()[i*8+j];
        }
    }

    return output;
}

void BinaryUtils::writeToFile(const std::string fileName, const char* bytes, const ulong byteSize, const bool append)
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
}

auto BinaryUtils::numToByteArray(const float num)
{
    union
    {
        float input;
        int output;
    } data;
    data.input = num;
    return numToByteArray(data.output);
}

ulong BinaryUtils::byteArrayToUint(std::shared_ptr<Byte> arr, int arrLen)
{
    ulong num = 0;

    for (int i = 0; i < arrLen; i++)
    {
        num |= (ulong) arr.get()[i] << ((arrLen-i-1)*8);    // going from right to left
    }

    return num;
}

float BinaryUtils::byteArrayToFloat(std::shared_ptr<Byte> arr, int arrLen)
{
    if (arrLen != 4)
    {
        std::logic_error("Can only convert char arrays of length 4 to float.");
    }

    union {
        unsigned int x;
        float f;
    } temp;
    temp.x = byteArrayToUint(arr, arrLen);
    return temp.f;
}

auto BinaryUtils::charInfoToByteArray(const CharInfo ci)
{
    std::array<Byte, sizeof(CharInfo)> result;
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

auto BinaryUtils::byteArrayToBitArray(std::shared_ptr<Byte> input, int inputLen)
{
    std::vector<bool> result;
    result.reserve(inputLen);
    for (int i = 0; i < inputLen; i++)
    {
        char c = input.get()[i];
        for (int j = 0; j < 8; j++)
        {
            result[8*i+j] = (c >> (7-j)) & 0b1;
        }
    }
    return result;
}
