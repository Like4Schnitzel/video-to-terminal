#include "VTDIDecoder.hpp"

VTDIDecoder::VTDIDecoder(std::string path)
{
    std::ifstream vtdiFile (path);
    if (!vtdiFile.good())
    {
        throw std::runtime_error("Cannot open file to read from.");
    }
    std::cout << "File opened succesfully.\n";
    vtdiFile.close();
    vtdiPath = path;
}

template <typename T>
T applyAssign(T num, int* index, char*& sib)
{
    float comparisonTool;

    int bytes = sizeof(T);
    int oldIndex = *index;
    *index += bytes;
    char* sub = VariousUtils::subArray(sib, oldIndex, *index);
    T result;
    if (typeid(T) == typeid(comparisonTool))
    {
        result = BinaryUtils::charArrayToFloat(sub, bytes);
    }
    else
    {
        result = (T)BinaryUtils::charArrayToUint(sub, bytes);
    }

    free(sub);
    return result;
}

void VTDIDecoder::getStaticInfo()
{
    // these are taken from the spec
    const int expectedSig[] = {86, 84, 68, 73};

    char* staticInfoBytes = (char*)malloc(6);
    std::ifstream vtdiFile (vtdiPath);
    vtdiFile.read(staticInfoBytes, 6);
    int index;

    for (index = 0; index < 4; index++)
    {
        if (staticInfoBytes[index] != expectedSig[index])
        {
            std::runtime_error("File signature does not match expected signature.");
        }
    }
    std::cout << "File signature is correct!\n";

    version = applyAssign(version, &index, staticInfoBytes);

    switch (version)
    {
        case 1:
        {
            this->staticByteSize = 38;
            break;
        }
        
        default:
        {
            std::runtime_error("File version not supported by decoder!");
            break;
        }
    }

    vtdiFile.read(staticInfoBytes, staticByteSize-6);
    index = 0;

    auto args = std::make_tuple(
        &frameCount, &FPS, &vidWidth, &vidHeight, &uncompressedSize, &compressedSize
    );
    std::apply([&](auto&... args) {
        (..., (*args = applyAssign(*args, &index, staticInfoBytes)));
    }, args);

    vtdiFile.close();
}

int VTDIDecoder::getVersion()
{
    return version;
}

uint32_t VTDIDecoder::getFrameCount()
{
    return frameCount;
}

float VTDIDecoder::getFPS()
{
    return FPS;
}

int VTDIDecoder::getVidWidth()
{
    return vidWidth;
}

int VTDIDecoder::getVidHeight()
{
    return vidHeight;
}
