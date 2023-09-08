#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <stdexcept>
#include <iostream>

class VideoTranscoder 
{
    private:
        cv::Mat frame;
        cv::VideoCapture vidCap;
        std::string vidPath;
        int vidHeight;
        int vidWidth;
    public:
        VideoTranscoder(std::string);
        ~VideoTranscoder();
        cv::Mat getFrame();
        int getFPS();
};
