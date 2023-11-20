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

SmartPtr<Byte> BinaryUtils::numToByteArray(const float num)
{
    union
    {
        float input;
        int output;
    } data;
    data.input = num;
    return numToByteArray(data.output);
}

ulong BinaryUtils::byteArrayToUint(SmartPtr<Byte> arr)
{
    ulong num = 0;

    for (int i = 0; i < arr.getSize(); i++)
    {
        num |= (ulong) arr.get(i) << ((arr.getSize()-i-1)*8);    // going from right to left
    }

    return num;
}

float BinaryUtils::byteArrayToFloat(SmartPtr<Byte> arr)
{
    if (arr.getSize() != 4)
    {
        std::logic_error("Can only convert char arrays of length 4 to float.");
    }

    union {
        unsigned int x;
        float f;
    } temp;
    temp.x = byteArrayToUint(arr);
    return temp.f;
}

SmartPtr<Byte> BinaryUtils::charInfoToByteArray(const CharInfo ci)
{
    SmartPtr<Byte> result = SmartPtr<Byte>(sizeof(CharInfo));
    int index = 0;

    for (int i = 0; i < 3; i++)
    {
        result.set(index, ci.foregroundRGB[i]);
        index++;
    }

    for (int i = 0; i < 3; i++)
    {
        result.set(index, ci.backgroundRGB[i]);
        index++;
    }

    result.set(index, ci.chara);

    return result;
}

SmartPtr<bool> BinaryUtils::byteArrayToBitArray(SmartPtr<Byte> input)
{
    SmartPtr<bool> result = SmartPtr<bool>(input.getSize()*sizeof(char));
    for (int i = 0; i < input.getSize(); i++)
    {
        char c = input.get(i);
        for (int j = 0; j < 8; j++)
        {
            result.set(8*i+j, (c >> (7-j)) & 0b1);
        }
    }
    return result;
}
