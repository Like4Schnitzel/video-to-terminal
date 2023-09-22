#include "VTDIDecoder.hpp"

VTDIDecoder::VTDIDecoder(std::string path)
{
    this->version = 0;

    this->vtdiFile.open(path);
    if (!vtdiFile.good())
    {
        throw std::runtime_error("Cannot open file to read from.");
    }
    std::cout << "File opened succesfully.\n";
    vtdiFile.close();
    vtdiFile.clear();
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
    vtdiFile.open(vtdiPath);
    vtdiFile.read(staticInfoBytes, 6);
    int index;

    for (index = 0; index < 4; index++)
    {
        if (staticInfoBytes[index] != expectedSig[index])
        {
            throw std::runtime_error("File signature does not match expected signature.");
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
            throw std::runtime_error("File version not supported by decoder!");
            break;
        }
    }

    const int remainingBytes = staticByteSize - 6;
    staticInfoBytes = (char*)malloc(remainingBytes);
    vtdiFile.read(staticInfoBytes, remainingBytes);
    index = 0;

    auto args = std::make_tuple(
        &frameCount, &FPS, &vidWidth, &vidHeight, &uncompressedSize, &compressedSize
    );
    std::apply([&](auto&... args) {
        (..., (*args = applyAssign(*args, &index, staticInfoBytes)));
    }, args);

    free(staticInfoBytes);
    vtdiFile.close();
    vtdiFile.clear();
}

void VTDIDecoder::playVideo()
{
    vtdiFile.open(vtdiPath);
    // move past the static bytes
    vtdiFile.seekg(staticByteSize);

    if (this->version == 0)
    {
        throw std::runtime_error("It seems static info hasn't been initialized yet. Try running VTDIDecoder.getStaticInfo()");
    }

    int* terminalDimensions = VariousUtils::getTerminalDimensions();
    this->terminalWidth = terminalDimensions[0];
    this->terminalHeight = terminalDimensions[1];
    free(terminalDimensions);

    if (terminalWidth < vidWidth || terminalHeight < vidHeight)
    {
        std::stringstream errorMessage;
        errorMessage << "The terminal's size (" << terminalWidth << "x" << terminalHeight << 
        ") isn't big enough to display the video (" << vidWidth << "x" << vidHeight <<  ").";
        throw std::runtime_error(errorMessage.str());
    }

    vtdiFile.close();
    vtdiFile.clear();
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
