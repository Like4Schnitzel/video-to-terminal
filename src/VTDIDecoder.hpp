#pragma once

#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>
#include "CharInfoStruct.hpp"
#include "VariousUtils.hpp"
#include "BinaryUtils.hpp"
#include "BitStream.hpp"

class VTDIDecoder {
    private:
        int staticByteSize;
        std::string vtdiPath;
        std::ifstream vtdiFile;
        BitStream inBits;
        CharInfo* currentFrame;
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
        void readNextFrame();

        int getVersion();
        uint32_t getFrameCount();
        float getFPS();
        int getVidWidth();
        int getVidHeight();
};
