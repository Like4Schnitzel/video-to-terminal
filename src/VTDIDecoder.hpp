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
        int vidWidth;
        int vidHeight;
        int terminalWidth;
        int terminalHeight;
        float FPS;
        int version;
    public:
        VTDIDecoder(std::string path);
        void getStaticInfo();

        int getVersion();
        uint32_t getFrameCount();
};
