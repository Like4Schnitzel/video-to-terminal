#include "VideoTranscoder.hpp"
#include <vector>

VideoTranscoder::VideoTranscoder(std::string path)
{
    vidPath = path;
    std::cout << "Attempting to open \"" << path << "\".\n";
    vidCap.open(vidPath);
    if (!vidCap.isOpened())
    {
        throw std::invalid_argument("The video at the provided path could not be read.");
    }
    std::cout << "\"" << path << "\" opened successfully.\n";

    vidWidth = vidCap.get(cv::CAP_PROP_FRAME_WIDTH);
    vidHeight = vidCap.get(cv::CAP_PROP_FRAME_HEIGHT);
    std::cout << "Video dimensions: " << vidWidth << "x" << vidHeight << "\n";
}

VideoTranscoder::~VideoTranscoder()
{
    vidCap.release();
}

cv::Mat VideoTranscoder::getFrame()
{
    vidCap >> frame;
    return frame;
}

float VideoTranscoder::getFPS()
{
    return vidCap.get(cv::CAP_PROP_FPS);
}

bool* int8ToBitArray(uint8_t num)
{
    bool* arr = (bool*)malloc(sizeof(bool) * 8);
    for (int i = 7; i >= 0; i--)
    {
        arr[i] = num % 2;
        num /= 2;
    }
    return arr;
}

void push_array(std::vector<bool> vec, bool* arr, int arrLen)
{
    for (int i = 0; i < arrLen; i++) {
        vec.push_back(arr[i]);
    }
}

void VideoTranscoder::transCodeFile()
{
    std::vector<bool> stdiContent = std::vector<bool>();
    push_array(stdiContent, int8ToBitArray(86), 8);
}
