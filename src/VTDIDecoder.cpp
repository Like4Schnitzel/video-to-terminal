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
T applyAssign(T num, int& index, const Byte* sib)
{
    int bytes = sizeof(T);
    int oldIndex = index;
    index += bytes;
    auto sub = VariousUtils::subArray(sib, oldIndex, index);
    T result;
    if (typeid(T) == typeid(float))
    {
        result = BinaryUtils::byteArrayToFloat(sub.data(), sub.size());
    }
    else
    {
        result = (T)BinaryUtils::byteArrayToUint(sub.data(), sub.size());
    }

    return result;
}

void VTDIDecoder::getStaticInfo()
{
    // these are taken from the spec
    const int expectedSig[] = {86, 84, 68, 73};

    std::unique_ptr<Byte[]> staticInfoBytes = std::make_unique<Byte[]>(6);
    vtdiFile.open(vtdiPath);
    vtdiFile.read((char*)staticInfoBytes.get(), 6);
    int index;

    for (index = 0; index < 4; index++)
    {
        if (staticInfoBytes.get()[index] != expectedSig[index])
        {
            throw std::runtime_error("File signature does not match expected signature.");
        }
    }
    std::cout << "File signature is correct!\n";

    version = applyAssign(version, std::ref(index), staticInfoBytes.get());

    switch (version)
    {
        case 2:
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
    staticInfoBytes = std::make_unique_for_overwrite<Byte[]>(remainingBytes);
    vtdiFile.read((char*)staticInfoBytes.get(), remainingBytes);
    index = 0;

    auto args = std::make_tuple(
        &frameCount, &FPS, &vidWidth, &vidHeight, &uncompressedSize, &compressedSize
    );
    std::apply([&](auto&... args) {
        (..., (*args = applyAssign(*args, std::ref(index), staticInfoBytes.get())));
    }, args);

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

    auto terminalDimensions = VariousUtils::getTerminalDimensions();
    this->terminalWidth = terminalDimensions[0];
    this->terminalHeight = terminalDimensions[1];

    if (terminalWidth < vidWidth || terminalHeight < vidHeight)
    {
        std::stringstream errorMessage;
        errorMessage << "The terminal's size (" << terminalWidth << "x" << terminalHeight << 
        ") isn't big enough to display the video (" << vidWidth << "x" << vidHeight <<  ").";
        //throw std::runtime_error(errorMessage.str());
    }

    this->currentFrame = std::make_unique<CharInfo[]>(terminalWidth*terminalHeight);
    const int nanoSecondsPerFrame = 1000000000/this->FPS;

    std::cout << "\x1B[2J\x1B[H";    // clear screen and move to 0,0
    for (uint32_t i = 0; i < this->frameCount; i++)
    {
        auto startTime = std::chrono::system_clock::now();
        readAndDisplayNextFrame(true, false);
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
            auto ciAtLocation = this->currentFrame.get()[i*vidWidth+j];
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

void VTDIDecoder::readAndDisplayNextFrame(bool display, bool save)
{
    std::stringstream outputBuffer;
    
    vtdiFile.read(buffer.data(), 1);
    //int bytePos = vtdiFile.tellg();
    if (buffer[0] == 1)   // frame hasn't changed from the last one, continue to next frame
        return;
    else if (buffer[0] != 0)
        throw std::runtime_error("At the start of a frame there should only be either 0 or 1, not " + std::to_string(buffer[0]) + ".");

    int endMarker;
    std::string fgColorSetter;
    fgColorSetter.reserve(20);
    std::string bgColorSetter;
    bgColorSetter.reserve(20);
    do
    {
        fgColorSetter = "\x1B[38;2";

        vtdiFile.read(buffer.data(), 7);
        CharInfo current;
        for (int i = 0; i < 3; i++)
        {
            uint8_t byteNum = buffer[i];
            current.foregroundRGB[i] = byteNum;
            fgColorSetter += ";" + std::to_string(byteNum);
        }
        fgColorSetter += "m";

        bgColorSetter = "\x1B[48;2";
        for (int i = 0; i < 3; i++)
        {
            uint8_t byteNum = buffer[i+3];
            current.backgroundRGB[i] = byteNum;
            bgColorSetter += ";" + std::to_string(byteNum);
        }
        bgColorSetter += "m";

        current.chara = buffer[6];

        do
        {
            vtdiFile.read(buffer.data(), 1);
            endMarker = buffer[0];

            if (endMarker == 0)  // rectangle
            {
                std::array<int, 4> corners;

                for (int i = 0; i < 4; i++)
                {
                    vtdiFile.read(buffer.data(), sizeof(uint16_t));
                    corners[i] = BinaryUtils::byteArrayToUint(
                        (Byte*) buffer.data(),
                        sizeof(uint16_t)
                    );
                }

                std::string moveRight = "\x1B[" + std::to_string(corners[0]) + "C";
                if (display)
                {
                    outputBuffer << "\x1B[H"   // move cursor to top left
                                << "\x1B[" + std::to_string(corners[1]) + "B"    // move cursor down
                                << moveRight;
                }

                for (int y = corners[1]; y <= corners[3]; y++)
                {
                    outputBuffer << fgColorSetter << bgColorSetter;
                    for (int x = corners[0]; x <= corners[2]; x++)
                    {
                        if (save)
                        {
                            int matIndex = y*vidWidth+x;
                            for (int i = 0; i < 3; i++)
                            {
                                currentFrame.get()[matIndex].foregroundRGB[i] = current.foregroundRGB[i];
                                currentFrame.get()[matIndex].backgroundRGB[i] = current.backgroundRGB[i];
                            }
                            currentFrame.get()[matIndex].chara = current.chara;
                        }

                        if (display)
                        {
                            outputBuffer << VariousUtils::numToUnicodeBlockChar(current.chara);
                        }
                    }
                    if (display)
                    {
                        outputBuffer << "\x1B[0m"  // unset color
                                    << "\x1B[E"   // move cursor to start of next line
                                    << moveRight;
                    }
                }
            }

            else if (endMarker == 1)    // position
            {
                std::array<int, 2> corners;

                for (int i = 0; i < 2; i++)
                {
                    vtdiFile.read(buffer.data(), sizeof(uint16_t));
                    corners[i] = BinaryUtils::byteArrayToUint(
                        (Byte*) buffer.data(),
                        sizeof(uint16_t)
                    );
                }

                if (save)
                {
                    int matIndex = corners[1]*vidWidth+corners[0];
                    for (int i = 0; i < 3; i++)
                    {
                        currentFrame.get()[matIndex].foregroundRGB[i] = current.foregroundRGB[i];
                        currentFrame.get()[matIndex].backgroundRGB[i] = current.backgroundRGB[i];
                    }
                    currentFrame.get()[matIndex].chara = current.chara;
                }

                if (display)
                {
                    outputBuffer << "\x1B[H"   // move cursor to top left
                                << "\x1B[" + std::to_string(corners[1]) + "B"    // move cursor down
                                << "\x1B[" + std::to_string(corners[0]) + "C"    // move cursor right
                                << fgColorSetter << bgColorSetter
                                << VariousUtils::numToUnicodeBlockChar(current.chara);
                }
            }
        } while(endMarker < 2); // 0 for rect, 1 for pos, 2 for end of CI

    } while(endMarker < 3); // marks the end of the frame

    std::cout << outputBuffer.str();
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
