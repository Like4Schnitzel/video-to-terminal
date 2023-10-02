#include "VideoTranscoder.hpp"
#include "BinaryUtils.hpp"
#include "VariousUtils.hpp"

VideoTranscoder::VideoTranscoder(const std::string path, const uint16_t terminalWidth, const uint16_t terminalHeight, const uint32_t memoryCapacity)
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

    int frameSize = sizeof(CharInfo) * terminalHeight * terminalWidth;
    if (memoryCapacity < frameSize)
    {
        throw std::invalid_argument("Memory cap must be enough to store at least one frame (" + std::to_string(frameSize) + "B).");
    }
    keepInMemory = memoryCapacity;
    std::cout << "Memory capacity: " << memoryCapacity << "\n";
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
    char* bundle = (char*)malloc(keepInMemory);

    uint32_t bundleIndex = 0;
    int frameIndex = 0;
    double progress = -1;
    // writing transcoded video bytes
    std::cout << "Transcoding frames...\n";
    for (vidCap>>frame; !frame.empty(); vidCap>>frame)
    {
        // progress update
        double newProgress = (int)((double) frameIndex / vidFrames * 10000) / 10000.;  // round to 4 digits
        if (newProgress != progress)
        {
            progress = newProgress;
            std::cout << progress*100 << "\% done...    \r";
        }
        frameIndex++;

        CharInfo* frameChars = transcodeFrame();
        for (int i = 0; i < totalTerminalChars; i++)
        {
            char* charInfoBytes = BinaryUtils::charInfoToCharArray(frameChars[i]);
            for (int j = 0; j < sizeof(CharInfo); j++)
            {
                bundle[bundleIndex] = charInfoBytes[j];
                bundleIndex++;
            }
            free(charInfoBytes);
        }
        free(frameChars);

        // only compress once the memory cap or the last frame is reached
        if (bundleIndex + totalFrameBytes > keepInMemory || frameIndex == vidFrames)
        {
            CharArrayWithSize compressedFrameBytes = BinaryUtils::compressBytes(bundle, bundleIndex);
            // write compressed size
            BinaryUtils::writeToFile(vtdiFilePath, BinaryUtils::numToCharArray((uint32_t)compressedFrameBytes.size), sizeof(uint32_t), true, true);
            // and uncompressed size
            BinaryUtils::writeToFile(vtdiFilePath, BinaryUtils::numToCharArray(bundleIndex), sizeof(uint32_t), true, true);
            // and finally the compressed data
            BinaryUtils::writeToFile(vtdiFilePath, compressedFrameBytes.arr, compressedFrameBytes.size, true);

            bundleIndex = 0;
        }
    }
    std::cout << "100\% done!     \n";
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
