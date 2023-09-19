#pragma once

#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>
#include "CharInfoStruct.hpp"

class VTDIDecoder {
    private:
        std::string vtdiPath;
        CharInfo** currentFrame;
        uint32_t frameCount;
        int vidWidth;
        int vidHeight;
        int terminalWidth;
        int terminalHeight;
    public:
        VTDIDecoder(std::string path);
};
