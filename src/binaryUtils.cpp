#include "binaryUtils.hpp"

bool* BinaryUtils::uint8ToBitArray(uint8_t num)
{
    bool* arr = (bool*)malloc(sizeof(bool) * 8);
    for (int i = 7; i >= 0; i--)
    {
        arr[i] = num % 2;
        num /= 2;
    }
    return arr;
}

void BinaryUtils::push_array(std::vector<bool> vec, bool* arr, int arrLen)
{
    for (int i = 0; i < arrLen; i++) {
        vec.push_back(arr[i]);
    }
}