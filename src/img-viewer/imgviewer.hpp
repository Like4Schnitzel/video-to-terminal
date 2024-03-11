#pragma once

#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <error.h>
#include <math.h>
#include "../libs/variousutils.hpp"
#include "../libs/charinfo.hpp"

namespace vtt {

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

        /// @brief Get the width of the original image file.
        int getPixelWidth();
        /// @brief Get the height of the original image file.
        int getPixelHeight();

        /// @brief Get the width of the transcoded image.
        int getTerminalWidth();
        /// @brief Get the height of the transcoded image.
        int getTerminalHeight();
};

}
