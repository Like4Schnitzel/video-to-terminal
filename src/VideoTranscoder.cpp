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
}

cv::Mat VideoTranscoder::getFrame()
{
    return frame;
}
