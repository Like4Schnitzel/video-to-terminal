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

int rfind(std::string str, char c)
{
    int i;
    for (i = str.length()-1; i >= 0; i--)
    {
        if (str[i] == c)
        {
            break;
        }
    }
    return i;
}

void VideoTranscoder::transCodeFile()
{
    const uint16_t versionNumber = 1;

    std::vector<bool> stdiContent = std::vector<bool>();
    BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(uint8_t(86)), 8);
    BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(uint8_t(84)), 8);
    BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(uint8_t(68)), 8);
    BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(uint8_t(73)), 8);
    //std::cout << "Amount of written bits: " << stdiContent.size() << "\n";

    const std::string vtdiFilePath = vidPath.substr(0, rfind(vidPath, '.')) + ".vtdi";
    BinaryUtils::writeToFile(vtdiFilePath, stdiContent);
    std::cout << "Transcoded file written succesfully to \"" << vtdiFilePath <<"\"!\n";
}
