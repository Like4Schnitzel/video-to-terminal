#include "VideoTranscoder.hpp"

VideoTranscoder::VideoTranscoder(std::string path)
{
    vidPath = path;
    vidCap = cv::VideoCapture(vidPath);
    // set one frame for testing
    vidCap>>frame;
    if (!frame.data)
    {
        throw std::invalid_argument("The video at the provided path could not be read.");
    }
}

cv::Mat VideoTranscoder::getFrame()
{
    return frame;
}
