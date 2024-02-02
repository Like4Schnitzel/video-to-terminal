#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <stdexcept>
#include <iostream>
#include <math.h>
#include <thread>
#include <future>
#include "../libs/CharInfoStruct.hpp"
#include "../libs/BinaryUtils.hpp"
#include "../libs/VariousUtils.hpp"

class VideoTranscoder 
{
    private:
        cv::Mat frame;
        cv::VideoCapture vidCap;
        std::string vidPath;
        std::string vtdiPath;
        uint16_t vidHeight;
        uint16_t vidWidth;
        uint32_t vidFrames;
        uint16_t vidTHeight;
        uint16_t vidTWidth;
        float vidFPS;
    public:
        VideoTranscoder(const std::string path, const std::string vtdiPath, const uint16_t terminalWidth, const uint16_t terminalHeight);
        ~VideoTranscoder();
        cv::Mat getFrame();
        void transcodeFile();
        std::shared_ptr<CharInfo []> transcodeFrame();
        std::vector<Byte> compressFrame(std::shared_ptr<CharInfo[]> currentFrame, std::shared_ptr<CharInfo[]> prevFrame);
};
