#include "VideoTranscoder.hpp"
#include "BinaryUtils.hpp"

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

void VideoTranscoder::transcodeFile()
{
    const uint16_t versionNumber = 1;   // change if updates to the file format are made

    std::vector<bool> stdiContent = std::vector<bool>();

    // write all the pre-video information to the file
    auto args = std::make_tuple(
        uint8_t(86), uint8_t(84), uint8_t(68), uint8_t(73), versionNumber, vidFrames, vidFPS, vidWidth, vidHeight, vidTWidth, vidTHeight
    );
    std::apply([&](auto... args) {
        (..., BinaryUtils::pushArray(&stdiContent, BinaryUtils::numToBitArray(args), sizeof(args)*CHAR_BIT));
    }, args);

    // settings constants for video bit writing
    const int totalTerminalChars = vidTWidth * vidTHeight;
    const int bitsInCharInfo = sizeof(CharInfo) * CHAR_BIT;
    const ulong totalVideoBits = (vidTWidth * vidTHeight + 1) * vidFrames * bitsInCharInfo;
    CharInfo ender;
    ender.foregroundRGB = {0, 0, 0};
    ender.backgroundRGB = {0, 0, 0};
    ender.chara = 32;
    bool* enderBits = BinaryUtils::charInfoToBitArray(ender);
    bool* videoBits = (bool*)malloc(totalVideoBits);
    int videoBitIndex = 0;
    int frameIndex = 0;
    double progress = -1;
    // writing transcoded video bits
    std::cout << "Transcoding frames...\n";
    for (vidCap>>frame; !frame.empty(); vidCap>>frame)
    {
        double newProgress = (int)((double) frameIndex / vidFrames * 100) / 100.;  // round to 2 digits
        if (newProgress != progress)
        {
            progress = newProgress;
            std::cout << "test?";
            std::cout << progress*100;
        }
        frameIndex++;
        CharInfo* frameChars = transcodeFrame();
        for (int i = 0; i < totalTerminalChars; i++)
        {
            bool* charInfoBits = BinaryUtils::charInfoToBitArray(frameChars[i]);
            for (int j = 0; j < bitsInCharInfo; j++)
            {
                videoBits[videoBitIndex] = charInfoBits[j];
                videoBitIndex++;
            }
            free(charInfoBits);
        }
        free(frameChars);
        // write frame ender bit sequence
        for (int i = 0; i < bitsInCharInfo; i++)
        {
            videoBits[videoBitIndex] = enderBits[i];
            videoBitIndex++;
        }
    }
    free(enderBits);

    if (totalVideoBits != videoBitIndex)
    {
        std::cout << "Something went wrong! videoBitIndex is " << videoBitIndex << " when it should be " << totalVideoBits << "\n";
    }

    const BoolArrayWithSize compressedVideoBits = BinaryUtils::compressBits(videoBits, videoBitIndex);
    std::cout << "Compressed video bits from " << videoBitIndex << "b to " << compressedVideoBits.size << "b\n";
    free(videoBits);
    BinaryUtils::pushArray(&stdiContent, compressedVideoBits.arr, compressedVideoBits.size);

    const std::string vtdiFilePath = vidPath.substr(0, rfind(vidPath, '.')) + ".vtdi";
    BinaryUtils::writeToFile(vtdiFilePath, stdiContent);
    std::cout << "Wrote " << stdiContent.size()/8 << " bytes to \"" << vtdiFilePath <<"\"!\n";
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

int getTotalRGBDiff(cv::Vec3b v1, cv::Vec3b v2)
{
    int diff = 0;
    for (int i = 0; i < 3; i++)
    {
        int tempDiff = v1[i] - v2[i];
        if (tempDiff < 0) tempDiff = -tempDiff;
        diff += tempDiff;
    }
    return diff;
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

        int totalRGBDiff = getTotalRGBDiff(avgForegroundRGB, avgBackgroundRGB);
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
