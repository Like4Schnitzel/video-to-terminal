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
T applyAssign(T num, int* index, Byte*& sib)
{
    float comparisonTool;

    int bytes = sizeof(T);
    int oldIndex = *index;
    *index += bytes;
    Byte* sub = VariousUtils::subArray(sib, oldIndex, *index);
    T result;
    if (typeid(T) == typeid(comparisonTool))
    {
        result = BinaryUtils::byteArrayToFloat(sub, bytes);
    }
    else
    {
        result = (T)BinaryUtils::byteArrayToUint(sub, bytes);
    }

    free(sub);
    return result;
}

void VTDIDecoder::getStaticInfo()
{
    // these are taken from the spec
    const int expectedSig[] = {86, 84, 68, 73};

    Byte* staticInfoBytes = (Byte*)malloc(6);
    vtdiFile.open(vtdiPath);
    vtdiFile.read((char*)staticInfoBytes, 6);
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
            this->staticByteSize = 18;
            break;
        }
        
        default:
        {
            throw std::runtime_error("File version not supported by decoder!");
            break;
        }
    }

    const int remainingBytes = staticByteSize - 6;
    staticInfoBytes = (Byte*)malloc(remainingBytes);
    vtdiFile.read((char*)staticInfoBytes, remainingBytes);
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

    BitStream inBits = BitStream(&vtdiFile, 5); // 5 byte buffer since we never need to read more than 4 bytes at once

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

    std::cout << "\x1B[2J"; // clear screen
    for (uint32_t i = 0; i < this->frameCount; i++)
    {
        readAndDisplayNextFrame(inBits);
    }

    vtdiFile.close();
    vtdiFile.clear();
}

void VTDIDecoder::readAndDisplayNextFrame(BitStream& inBits)
{
    bool* startBit = inBits.readBits(1);
    if (startBit[0] == 1)   // frame hasn't changed from the last one, continue to next frame
    {
        free(startBit);
        free(inBits.readBits(7));   // go through padded bits
        return;
    }
    free(startBit);

    bool* endMarker = nullptr;
    do
    {
        std::string ansiColorCodeSetter = "\x1B[38;2;";

        bool* byteBits;
        Byte* bytes;
        CharInfo current;
        for (int i = 0; i < 3; i++)
        {
            byteBits = inBits.readBits(8);
            bytes = BinaryUtils::bitArrayToByteArray(byteBits, 8);
            current.foregroundRGB[i] = bytes[0];
            ansiColorCodeSetter += std::to_string((int)(bytes[0]));
            ansiColorCodeSetter += ";";
            free(bytes);
            free(byteBits);
        }
        ansiColorCodeSetter += "m\x1B[48;2;";

        for (int i = 0; i < 3; i++)
        {
            byteBits = inBits.readBits(8);
            bytes = BinaryUtils::bitArrayToByteArray(byteBits, 8);
            current.backgroundRGB[i] = bytes[0];
            ansiColorCodeSetter += std::to_string((int)(bytes[0]));
            ansiColorCodeSetter += ";";
            free(bytes);
            free(byteBits);
        }
        ansiColorCodeSetter += "m";
        //std::cout << ansiColorCodeSetter;

        byteBits = inBits.readBits(8);
        bytes = BinaryUtils::bitArrayToByteArray(byteBits, 8);
        current.chara = bytes[0];
        free(bytes);
        free(byteBits);

        do
        {
            free(endMarker);
            endMarker = inBits.readBits(2);

            if (endMarker[0] == 0)
            {
                if (endMarker[1] == 0)  // rectangle
                {
                    int* corners = (int*)malloc(4*sizeof(int));

                    for (int i = 0; i < 4; i++)
                    {
                        byteBits = inBits.readBits(16);
                        bytes = BinaryUtils::bitArrayToByteArray(byteBits, 16);
                        corners[i] = BinaryUtils::byteArrayToUint(bytes, 2);
                        free(byteBits);
                        free(bytes);
                    }

                    std::string setCursorPos = "\x1B[";
                    setCursorPos += corners[1];
                    setCursorPos += ";";
                    setCursorPos += corners[0];
                    setCursorPos += "f";
                    std::cout << setCursorPos;

                    for (int y = corners[3]; y < corners[1]; y++)
                    {
                        std::cout << "\x1B[s";
                        for (int x = corners[0]; x < corners[2]; x++)
                        {
                            int matIndex = y*vidWidth+x;
                            for (int i = 0; i < 3; i++)
                            {
                                currentFrame[matIndex].foregroundRGB[i] = current.foregroundRGB[i];
                                currentFrame[matIndex].backgroundRGB[i] = current.backgroundRGB[i];
                            }
                            currentFrame[matIndex].chara = current.chara;

                            std::string c = VariousUtils::numToUnicodeBlockChar(current.chara);
                            std::cout << c;
                        }
                        std::cout << "\x1B[u\x1B[1B";
                    }

                    free(corners);
                }
                else    // position
                {
                    int* corners = (int*)malloc(2*sizeof(int));

                    for (int i = 0; i < 2; i++)
                    {
                        byteBits = inBits.readBits(16);
                        bytes = BinaryUtils::bitArrayToByteArray(byteBits, 16);
                        corners[i] = BinaryUtils::byteArrayToUint(bytes, 2);
                        free(byteBits);
                        free(bytes);
                    }

                    int matIndex = corners[1]*vidWidth+corners[0];
                    for (int i = 0; i < 3; i++)
                    {
                        currentFrame[matIndex].foregroundRGB[i] = current.foregroundRGB[i];
                        currentFrame[matIndex].backgroundRGB[i] = current.backgroundRGB[i];
                    }
                    currentFrame[matIndex].chara = current.chara;
                }
            }
        } while(endMarker[0] == 0); // 00 for rect, 01 for pos, 10 for end of CI

    } while(endMarker[1] == 0);    // 11 marks the end of the frame
    free(endMarker);

    // go through padding bits
    int bitStreamIndex = inBits.getIndex();
    if (bitStreamIndex > 0)
    {
        free(inBits.readBits(8-bitStreamIndex));
    }
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
