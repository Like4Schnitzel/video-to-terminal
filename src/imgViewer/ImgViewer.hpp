#pragma once

#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <error.h>
#include "../libs/VariousUtils.hpp"
#include "../libs/CharInfoStruct.hpp"

class ImgViewer {
    private:
        cv::Mat file;
        std::vector<CharInfo> transcodedFile;
        int width;
        int height;
    public:
        ImgViewer(const std::string path);
        void transcode(const int width, const int height);
        void print();
};
