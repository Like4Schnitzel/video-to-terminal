#include "VideoTranscoder.hpp"
#include "BinaryUtils.hpp"
#include "VariousUtils.hpp"

VideoTranscoder::VideoTranscoder(const std::string path, const uint16_t terminalWidth, const uint16_t terminalHeight)
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

void VideoTranscoder::transcodeFile()
{
    const std::string vtdiFilePath = vidPath.substr(0, VariousUtils::rfind(vidPath, '.')) + ".vtdi";
    const uint16_t versionNumber = 1;   // change if updates to the file format are made

    // write all the pre-video information to the file
    auto args = std::make_tuple(
        uint8_t(86), uint8_t(84), uint8_t(68), uint8_t(73), versionNumber, vidFrames, vidFPS, vidTWidth, vidTHeight
    );
    std::apply([&](auto... args)
    {
        (..., BinaryUtils::writeToFile(vtdiFilePath, (char*)BinaryUtils::numToCharArray(args), sizeof(args), true, true));
    }, args);

    // settings constants for video byte writing
    const int totalTerminalChars = vidTWidth * vidTHeight;
    const uint32_t totalFrameBytes = (vidTWidth * vidTHeight) * sizeof(CharInfo);
    CharInfo* previousFrameChars = nullptr;

    uint32_t frameBytesIndex = 0;
    int frameIndex = 0;
    double progress = -1;
    // writing transcoded video bytes
    std::cout << "Transcoding frames...\n";
    for (vidCap>>frame; !frame.empty(); vidCap>>frame)
    {
        // progress update
        int newProgress = (int)((double) frameIndex / vidFrames * 10000) / 100.;  // round to 4 digits
        if (newProgress != progress)
        {
            progress = newProgress;
            std::cout << progress << "\% done...    \r";
        }
        frameIndex++;

        CharInfo* frameChars = transcodeFrame();
        std::vector<bool> frameBits = compressFrame(frameChars, previousFrameChars);
        free(previousFrameChars);
        previousFrameChars = frameChars;
    }
    std::cout << "100\% done!     \n";
}

std::vector<bool> VideoTranscoder::compressFrame(CharInfo* currentFrame, CharInfo* prevFrame)
{
    const uint32_t arraySize = vidTWidth * vidTHeight; 
    std::vector<bool> result;

    std::map<ulong, bool*> compressedVals;

    // making bitmaps of all CharInfos
    for (uint32_t index = 0; index < arraySize; index++)
    {
        char* ciBytes = BinaryUtils::charInfoToCharArray(currentFrame[index]);
        ulong currentCIHash = BinaryUtils::charArrayToUint(ciBytes, sizeof(CharInfo));
        free(ciBytes);

        bool prevFrameExists = (prevFrame != nullptr);
        ulong prevCIHash;
        if (prevFrameExists)
        {
            ciBytes = BinaryUtils::charInfoToCharArray(prevFrame[index]);
            prevCIHash = BinaryUtils::charArrayToUint(ciBytes, sizeof(CharInfo));
            free(ciBytes);
        }

        // only proceed if the CI has changed from the last frame
        if (!prevFrameExists || currentCIHash != prevCIHash)
        {
            // if new charinfo, create bitmap
            if (compressedVals.count(currentCIHash) == 0)
            {
                bool* binMat = (bool*)malloc(arraySize*sizeof(bool));
                for (int i = 0; i < arraySize; i++)
                {
                    binMat[i] = 0;
                }
                
                compressedVals.insert({currentCIHash, binMat});
            }

            // mark occurence
            compressedVals[currentCIHash][index] = 1;
        }
    }

    // now compress the bitmaps
    for (std::map<ulong, bool*>::iterator it = compressedVals.begin(); it != compressedVals.end(); it++)
    {
        ulong ciHash = it->first;
        bool* bitmap = it->second;
        // append CI bits
        char* ciHashBytes = BinaryUtils::numToCharArray(ciHash);
        for (int i = 0; i < sizeof(CharInfo); i++) // start at the second byte, since CIs only have 7 but ulongs have 8
        {
            char c = ciHashBytes[i+1];
            for (int j = 0; j < 8; j++)
            {
                bool bit = (bool)((c >> (7-j)) & 1);
                result.push_back(bit);
            }
        }
        free(ciHashBytes);

        // write end of frame (0b11)
        result.push_back(1);
        result.push_back(1);
    }

    return result;
}

cv::Vec3b getAverageRGB(cv::Mat img)
{
    const int imageHeight = img.size().height;
    const int imageWidth = img.size().width;
    const int totalPixels = imageHeight * imageWidth;

    //std::cout << "imageHeight: " << imageHeight << "\nimageWidth: " << imageWidth << "\n";
    cv::Vec3b avrgRGB = {0, 0, 0};
    ulong totalRGB[3] = {0, 0, 0};

    if (imageHeight > 0 && imageWidth > 0)
    {
        //cv::imshow("frame", img);
        for (int i = 0; i < imageHeight; i++)
        {
            for (int j = 0; j < imageWidth; j++)
            {
                cv::Vec3b pixelBGRValues = img.at<cv::Vec3b>(cv::Point(i, j));
                //std::cout << "pixelBGRValues: " << (int)pixelBGRValues[0] << " " << (int)pixelBGRValues[1] << " " << (int)pixelBGRValues[2] << "\n";
                for (int k = 0; k < 3; k++)
                {
                    totalRGB[2-k] += pixelBGRValues[k];
                }
            }
        }

        for (int i = 0; i < 3; i++)
        {
            avrgRGB[i] = totalRGB[i] / totalPixels;
        }

        //std::cout << "Average RGB: " << (int)avrgRGB[0] << " " << (int)avrgRGB[1] << " " << (int)avrgRGB[2] << "\n";
        //cv::waitKey(0);
    }

    return avrgRGB;
}

int getRGBDiff(cv::Vec3b v1, cv::Vec3b v2)
{
    return (int) sqrt(pow(v1[0]-v2[0], 2) + pow(v1[1]-v2[1], 2) + pow(v1[2]-v2[2], 2));
}

CharInfo findBestBlockCharacter(cv::Mat img)
{
    CharInfo minDiffCharInfo;
    int minDiff = 257;
    const int imageHeight = img.size().height;
    const int imageWidth = img.size().width;
    int currentOption;
    cv::Mat foreground;
    cv::Mat background;
    // skip upper half since lower half can be used
    // loop through the lower eights
    const double eigthHeight = (double)imageHeight / 8;
    //std::cout << "eightHeight: " << eigthHeight << "\n";
    double currentHeight = imageHeight;
    //cv::imshow("bigger frame", img);
    for (currentOption = 1; currentOption < 8; currentOption++)
    {
        //std::cout << "currentOption: " << currentOption << "\ncurrentHeight: " << currentHeight << "\n";
        currentHeight -= eigthHeight;
        foreground = img(cv::Rect(0, (int)currentHeight, imageWidth, imageHeight-(int)currentHeight));
        //std::cout << "foreground dimensions: " << foreground.size().width << "x" << foreground.size().height << "\n";
        background = img(cv::Rect(0, 0, imageWidth, (int)currentHeight));
        //std::cout << "background dimensions: " << background.size().width << "x" << background.size().height << "\n";
        cv::Vec3b avgForegroundRGB = getAverageRGB(foreground);
        cv::Vec3b avgBackgroundRGB = getAverageRGB(background);

        int totalRGBDiff = getRGBDiff(avgForegroundRGB, avgBackgroundRGB);
        if (totalRGBDiff < minDiff)
        {
            minDiff = totalRGBDiff;
            minDiffCharInfo.foregroundRGB = avgForegroundRGB;
            minDiffCharInfo.backgroundRGB = avgBackgroundRGB;
            minDiffCharInfo.chara = currentOption;
        }
    }

    return minDiffCharInfo;
}

CharInfo* VideoTranscoder::transcodeFrame()
{
    CharInfo* frameInfo = (CharInfo*)malloc(sizeof(CharInfo) * vidTHeight * vidTWidth);
    int charIndex = 0;

    // downscale
    if (vidWidth >= vidTWidth)
    {
        const double widthPixelsPerChar = vidWidth / vidTWidth;
        const double heightPixelsPerChar = vidHeight / vidTHeight;

        //cv::imshow("biggest image", frame);
        //std::cout << "frame dimensions: " << frame.size().width << "x" << frame.size().height << "\n";
        for (double y = 0; y+heightPixelsPerChar < vidHeight; y += heightPixelsPerChar)
        {
            for (double x = 0; x+widthPixelsPerChar < vidWidth; x += widthPixelsPerChar)
            {
                cv::Mat framePart = this->frame(cv::Rect((int)x, (int)y, (int)widthPixelsPerChar, (int)heightPixelsPerChar));
                frameInfo[charIndex] = findBestBlockCharacter(framePart);
            }
        }
    }
    else
    {
        std::cout << "downscaling\n";
    }

    return frameInfo;
}