#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <stdexcept>
#include <iostream>
#include <math.h>
#include "CharInfoStruct.hpp"

class VideoTranscoder 
{
    private:
        cv::Mat frame;
        cv::VideoCapture vidCap;
        std::string vidPath;
        uint16_t vidHeight;
        uint16_t vidWidth;
        uint32_t vidFrames;
        uint16_t vidTHeight;
        uint16_t vidTWidth;
        float vidFPS;
        uint32_t keepInMemory;  // how many bytes of frame data to keep in memory before compressing and writing
    public:
        VideoTranscoder(const std::string path, const uint16_t terminalWidth, const uint16_t terminalHeight, const uint32_t memoryCapacity);
        ~VideoTranscoder();
        cv::Mat getFrame();
        void transcodeFile();
        CharInfo* transcodeFrame();
};
