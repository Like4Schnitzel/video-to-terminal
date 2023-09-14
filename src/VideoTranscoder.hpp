#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <stdexcept>
#include <iostream>
#include "BinaryUtils.hpp"

struct CharInfo {
    cv::Vec3b foregroundRGB;
    cv::Vec3b foregroundRGB;
    uint8_t chara;
};

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
    public:
        VideoTranscoder(std::string path, uint16_t terminalWidth, uint16_t terminalHeight);
        ~VideoTranscoder();
        cv::Mat getFrame();
        void transcodeFile();
        CharInfo* transcodeFrame();
};
