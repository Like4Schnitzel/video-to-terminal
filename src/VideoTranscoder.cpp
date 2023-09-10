#include "VideoTranscoder.hpp"

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



void VideoTranscoder::transCodeFile()
{
    std::vector<bool> stdiContent = std::vector<bool>();
    BinaryUtils::push_array(stdiContent, BinaryUtils::uint8ToBitArray(86), 8);
}
