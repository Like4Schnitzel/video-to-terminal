#include "binaryUtils.hpp"
#include <vector>

void BinaryUtils::pushArray(std::vector<bool>* vec, bool* arr, int arrLen)
{
    for (int i = 0; i < arrLen; i++) {
        (*vec).push_back(arr[i]);
        //std::cout << arr[i];
    }
    //std::cout << " pushed to vector.\nVector size is " << (*vec).size() << "\n";
}

void BinaryUtils::writeToFile(std::string fileName, std::vector<bool> bits)
{
    if (bits.size() % 8 != 0)
    {
        throw "Bits are not divisible by 8.";
    }
    const int byteSize = bits.size()/8;
    char* toWrite = (char*)malloc(byteSize);    // you need to write full bytes to files, 1 char is equal to 1 byte

    std::ofstream file(fileName, std::ios::out | std::ios::binary);
    if (!file.good())
    {
        throw "Cannot open file to write to.";
    }

    for (int i = 0; i < byteSize; i++)
    {
        toWrite[i] = 0;
        // write bits to char
        for (int j = 0; j < 8; j++)
        {
            toWrite[i] = toWrite[i] << 1 | bits[i*8 + j];   //push already written bits to the left by one, then write 0 or 1 on the very right
        }
        //std::cout << "Wrote bit " << toWrite[i] << " to file.\n";
    }

    file.write(toWrite, byteSize);
    file.close();
}
