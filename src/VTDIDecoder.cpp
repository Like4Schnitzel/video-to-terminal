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

void VTDIDecoder::getStaticInfo()
{
    // these are taken from the spec
    const int expectedSig[] = {86, 84, 68, 73};
    const int staticByteSize = 38;

    char* staticInfoBytes = (char*)malloc(staticByteSize);
    std::ifstream vtdiFile (vtdiPath);
    vtdiFile.read(staticInfoBytes, staticByteSize);
    int index;

    for (index = 0; index < 4; index++)
    {
        if (staticInfoBytes[index] != expectedSig[index])
        {
            std::runtime_error("File signature does not match expected signature.");
        }
    }
    std::cout << "File signature is correct!\n";

    version = BinaryUtils::charArrayToNum(VariousUtils::subArray(staticInfoBytes, index, index+2), 2);
    index += 2;

    vtdiFile.close();
}

int VTDIDecoder::getVersion()
{
    return version;
}
