#pragma once

#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include "CharInfoStruct.hpp"
#include "VariousUtils.hpp"
#include "BinaryUtils.hpp"
#include "BitStream.hpp"

class VTDIDecoder {
    private:
        int staticByteSize;
        std::string vtdiPath;
        std::ifstream vtdiFile;
        std::unique_ptr<CharInfo[]> currentFrame;
        uint32_t frameCount;
        uint16_t vidWidth;
        uint16_t vidHeight;
        int terminalWidth;
        int terminalHeight;
        float FPS;
        uint16_t version;
        ulong uncompressedSize;
        ulong compressedSize;
    public:
        VTDIDecoder(std::string path);
        void getStaticInfo();
        void playVideo();
        void readAndDisplayNextFrame(BitStream& inBits, bool display=true, bool save=true);
        void displayCurrentFrame();

        int getVersion();
        uint32_t getFrameCount();
        float getFPS();
        int getVidWidth();
        int getVidHeight();
};
