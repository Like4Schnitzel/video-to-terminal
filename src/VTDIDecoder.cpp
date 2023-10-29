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

VTDIDecoder::~VTDIDecoder()
{
    free(this->currentFrame);
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

    this->currentFrame = (CharInfo*)malloc(terminalWidth*terminalHeight*sizeof(CharInfo));
    const int nanoSecondsPerFrame = 1000000000/this->FPS;

    std::cout << "\x1B[2J\x1B[H";    // clear screen and move to 0,0
    for (uint32_t i = 0; i < this->frameCount; i++)
    {
        auto startTime = std::chrono::system_clock::now();
        readAndDisplayNextFrame(inBits, true);
        std::this_thread::sleep_until(startTime + std::chrono::nanoseconds(nanoSecondsPerFrame));
    }
    std::cout << "\x1B[H" << "\x1B[" + std::to_string(vidHeight+1) + "B"    // move cursor below the video
              << "\x1B[0m";

    vtdiFile.close();
    vtdiFile.clear();
}

void VTDIDecoder::displayCurrentFrame()
{
    std::cout << "\x1B[2J\x1B[H";
    for (int i = 0; i < this->vidHeight; i++)
    {
        for (int j = 0; j < this->vidWidth; j++)
        {
            auto ciAtLocation = this->currentFrame[i*vidWidth+j];
            std::string fgColorSetter = "\x1B[38;2";
            std::string bgColorSetter = "\x1B[48;2";
            for (int k = 0; k < 3; k++)
            {
                fgColorSetter += ";" + std::to_string((int)ciAtLocation.foregroundRGB[k]);
                bgColorSetter += ";" + std::to_string((int)ciAtLocation.backgroundRGB[k]);
            }
            fgColorSetter += "m";
            bgColorSetter += "m";

            std::cout << fgColorSetter << bgColorSetter << VariousUtils::numToUnicodeBlockChar(ciAtLocation.chara);
        }
        std::cout << "\x1B[0m\n";
    }
}

void VTDIDecoder::readAndDisplayNextFrame(BitStream& inBits, bool display)
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
        std::string fgColorSetter = "\x1B[38;2";

        bool* byteBits;
        Byte* bytes;
        CharInfo current;
        for (int i = 0; i < 3; i++)
        {
            byteBits = inBits.readBits(8);
            bytes = BinaryUtils::bitArrayToByteArray(byteBits, 8);
            current.foregroundRGB[i] = bytes[0];
            fgColorSetter += ";";
            fgColorSetter += std::to_string((int)bytes[0]);
            free(bytes);
            free(byteBits);
        }
        fgColorSetter += "m";

        std::string bgColorSetter = "\x1B[48;2";
        for (int i = 0; i < 3; i++)
        {
            byteBits = inBits.readBits(8);
            bytes = BinaryUtils::bitArrayToByteArray(byteBits, 8);
            current.backgroundRGB[i] = bytes[0];
            bgColorSetter += ";";
            bgColorSetter += std::to_string((int)bytes[0]);
            free(bytes);
            free(byteBits);
        }
        bgColorSetter += "m";

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

                    std::string moveRight = "\x1B[" + std::to_string(corners[0]) + "C";
                    if (display)
                    {
                        std::cout << "\x1B[H"   // move cursor to top left
                                  << "\x1B[" + std::to_string(corners[1]) + "B"    // move cursor down
                                  << moveRight;
                    }

                    for (int y = corners[1]; y <= corners[3]; y++)
                    {
                        std::cout << fgColorSetter << bgColorSetter;
                        for (int x = corners[0]; x <= corners[2]; x++)
                        {
                            int matIndex = y*vidWidth+x;
                            for (int i = 0; i < 3; i++)
                            {
                                currentFrame[matIndex].foregroundRGB[i] = current.foregroundRGB[i];
                                currentFrame[matIndex].backgroundRGB[i] = current.backgroundRGB[i];
                            }
                            currentFrame[matIndex].chara = current.chara;

                            if (display)
                            {
                                std::cout << VariousUtils::numToUnicodeBlockChar(current.chara);
                            }
                        }
                        if (display)
                        {
                            std::cout << "\x1B[0m"  // unset color
                                      << "\x1B[E"   // move cursor to start of next line
                                      << moveRight;
                        }
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

                    if (display)
                    {
                        std::cout << "\x1B[H"   // move cursor to top left
                                  << "\x1B[" + std::to_string(corners[1]) + "B"    // move cursor down
                                  << "\x1B[" + std::to_string(corners[0]) + "C"    // move cursor right
                                  << fgColorSetter << bgColorSetter
                                  << VariousUtils::numToUnicodeBlockChar(current.chara);
                    }
                }
            }
        } while(endMarker[0] == 0); // 00 for rect, 01 for pos, 10 for end of CI

    } while(endMarker[1] == 0); // 11 marks the end of the frame
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
