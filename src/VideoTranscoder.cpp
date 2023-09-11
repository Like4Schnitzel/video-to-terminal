#include "VideoTranscoder.hpp"

VideoTranscoder::VideoTranscoder(std::string path, uint16_t terminalWidth, uint16_t terminalHeight)
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
    vidFPS = vidCap.get(cv::CAP_PROP_FPS);
    std::cout << "Video FPS: " << vidFPS << "\n";
    vidFrames = vidCap.get(cv::CAP_PROP_FRAME_COUNT);
    std::cout << "Video Frames: " << vidFrames << "\n";
    vidTWidth = terminalWidth;
    vidTHeight = terminalHeight;
    std::cout << "Terminal dimensions: " << terminalWidth << "x" << terminalHeight << " characters\n";
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
    const uint16_t versionNumber = 1;   // change if updates to the file format are made

    std::vector<bool> stdiContent = std::vector<bool>();
    // file signature
    BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(uint8_t(86)), 8);
    BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(uint8_t(84)), 8);
    BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(uint8_t(68)), 8);
    BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(uint8_t(73)), 8);
    // version number
    BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(versionNumber), 16);
    // frames
    BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(vidFrames), 32);
    // FPS
    BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(vidFPS), 32);
    // width and height
    BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(vidWidth), 16);
    BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(vidHeight), 16);
    // terminal width and height
    BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(vidTWidth), 16);
    BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(vidTHeight), 16);

    const std::string vtdiFilePath = vidPath.substr(0, rfind(vidPath, '.')) + ".vtdi";
    BinaryUtils::writeToFile(vtdiFilePath, stdiContent);
    std::cout << "Wrote " << stdiContent.size()/8 << " bytes to \"" << vtdiFilePath <<"\"!\n";
}
