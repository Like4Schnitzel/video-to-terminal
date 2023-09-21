#pragma once

#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>
#include "CharInfoStruct.hpp"
#include "VariousUtils.hpp"
#include "BinaryUtils.hpp"

class VTDIDecoder {
    private:
        std::string vtdiPath;
        CharInfo** currentFrame;
        uint32_t frameCount;
        uint16_t vidWidth;
        uint16_t vidHeight;
        int terminalWidth;
        int terminalHeight;
        float FPS;
        uint16_t version;
    public:
        VTDIDecoder(std::string path);
        void getStaticInfo();

        int getVersion();
        uint32_t getFrameCount();
        float getFPS();
        int getVidWidth();
        int getVidHeight();
};
