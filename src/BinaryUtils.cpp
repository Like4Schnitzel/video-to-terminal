#include "BinaryUtils.hpp"

template <typename T>
char* bitArrayToCharArray(T bits, ulong len)
{
    if (len % 8 != 0)
    {
        throw std::logic_error("Bits are not divisible by 8.");
    }

    ulong byteSize = len / CHAR_BIT;
    char* output = (char*)malloc(byteSize);
    for (ulong i = 0; i < byteSize; i++)
    {
        output[i] = 0;
        // write bits to char
        for (int j = 0; j < 8; j++)
        {
            output[i] = output[i] << 1 | bits[i*8 + j];   //push already written bits to the left by one, then write 0 or 1 on the very right
        }
    }

    return output;
}

void BinaryUtils::pushArray(std::vector<bool>* vec, const bool* arr, const ulong arrLen)
{
    for (ulong i = 0; i < arrLen; i++) {
        (*vec).push_back(arr[i]);
        //std::cout << arr[i];
    }
    //std::cout << " pushed to vector.\nVector size is " << (*vec).size() << "\n";
}

void BinaryUtils::writeToFile(const std::string fileName, const std::vector<bool> bits)
{
    const ulong byteSize = bits.size()/CHAR_BIT;
    char* toWrite = bitArrayToCharArray(bits, bits.size());    // you need to write full bytes to files, 1 char is equal to 1 byte

    std::ofstream file(fileName, std::ios::out | std::ios::binary);
    if (!file.good())
    {
        throw std::runtime_error("Cannot open file to write to.");
    }
    file.write(toWrite, byteSize);
    free(toWrite);
    file.close();
}

bool* BinaryUtils::numToBitArray(const float num)
{
    // i have no idea how this works, but it saves the bitwise representation in the variable `bits`
    // taken from https://stackoverflow.com/a/474058
    union
    {
        float input;
        int output;
    } data;
    data.input = num;
    std::bitset<sizeof(float) * CHAR_BIT> bits(data.output);

    //std::cout << "float converted to " << bits << "\n";

    bool* out = (bool*)malloc(sizeof(bool)*32);
    for (int i = 0; i < 32; i++)
    {
        out[i] = bits[i];
    }
    return out;
}

bool* BinaryUtils::charInfoToBitArray(const CharInfo ci)
{
    bool* result = (bool*)malloc(sizeof(CharInfo)*CHAR_BIT);
    int index = 0;

    for (cv::Vec3b vec : {ci.foregroundRGB, ci.backgroundRGB})
    {
        for (int i = 0; i < 3; i++)
        {
            bool* numBits = numToBitArray(vec[i]);
            for (int j = 0; j < 8; j++)
            {
                result[index] = numBits[j];
                index++;
            }
            free(numBits);
        }
    }

    bool* numBits = numToBitArray(ci.chara);
    for (int i = 0; i < 8; i++)
    {
        result[index] = numBits[i];
        index++;
    }
    free(numBits);

    return result;
}

// most of this is taken from https://gist.github.com/arq5x/5315739
BoolArrayWithSize BinaryUtils::compressBits(const bool* input, const ulong inputLength)
{
    char* in = bitArrayToCharArray(input, inputLength);
    char* out = (char*)malloc(inputLength);
    z_stream defstream;
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;
    defstream.avail_in = inputLength;
    defstream.next_in = (Bytef*) in;
    defstream.avail_out = inputLength;
    defstream.next_out = (Bytef*) out;

    std::cout << "compressing...\n";
    deflateInit(&defstream, Z_BEST_COMPRESSION);
    deflate(&defstream, Z_FINISH);
    deflateEnd(&defstream);

    BoolArrayWithSize output;
    output.size = defstream.total_out*CHAR_BIT;
    bool* outBits = (bool*)malloc(output.size);
    for (ulong i = 0; i < defstream.total_out; i++)
    {
        bool* tempOutBitsChar = numToBitArray(out[i]);
        for (int j = 0; j < 8; j++)
        {
            outBits[8*i+j] = tempOutBitsChar[j];
        }
        free(tempOutBitsChar);
    }
    free(out);
    output.arr = outBits;

    return output;
}
