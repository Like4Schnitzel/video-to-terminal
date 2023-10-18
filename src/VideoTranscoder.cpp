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
        double newProgress = (int)((double) frameIndex / vidFrames * 10000) / 100.;  // round to 4 digits
        if (newProgress != progress)
        {
            progress = newProgress;
            std::cout << std::fixed << std::setprecision(2) << "\33[2K\r" << progress << "\% done..." << std::flush;
        }
        frameIndex++;

        CharInfo* frameChars = transcodeFrame();
        std::vector<bool> frameBits = compressFrame(frameChars, previousFrameChars);
        // pad bits to full byte
        while (frameBits.size() % 8 != 0)
        {
            frameBits.push_back(0);
        }
        char* frameBytes = BinaryUtils::bitArrayToCharArray(frameBits, frameBits.size());
        BinaryUtils::writeToFile(vtdiFilePath, frameBytes, frameBits.size()/8, true);
        free(frameBytes);

        free(previousFrameChars);
        previousFrameChars = frameChars;
    }
    free(previousFrameChars);

    std::cout << "\33[2k\r" << "100\% done!\n" << std::flush;
}

int* findBiggestRectangle(bool* bitmap, int bitCount, int rowLength)
{
    int maxArea = 0;
    // -1 will be returned if nothing has been found
    int maxXpos1 = -1, maxYpos1, maxXpos2, maxYpos2; // coordinate pairs for the corners of the rectangle

    int* addedValsRow = (int*)malloc(rowLength*sizeof(int));
    for (int i = 0; i < rowLength; i++)
    {
        addedValsRow[i] = 0;
    }

    const int rows = bitCount / rowLength;
    for (int i = 0; i < rows; i++)
    {
        int* uniqueDepths = (int*)malloc(rowLength*sizeof(int));
        int uniqueDepthsCount = 0;

        // overlay values with above rows
        for (int j = 0; j < rowLength; j++)
        {
            if (bitmap[i*rowLength+j] == 0)
            {
                addedValsRow[j] = 0;
            }
            else
            {
                addedValsRow[j]++;
                if (VariousUtils::find(uniqueDepths, addedValsRow[j], 0, uniqueDepthsCount) == -1)
                {
                    uniqueDepths[uniqueDepthsCount] = addedValsRow[j];
                    uniqueDepthsCount++;
                }
            }
        }

        // find biggest rectangle
        for (int depthIndex = 0; depthIndex < uniqueDepthsCount; depthIndex++)
        {
            int area = 0;
            int currentDepth = uniqueDepths[depthIndex];
            int xpos1 = -1, ypos1, xpos2, ypos2 = i;

            for (int j = 0; j < rowLength; j++)
            {
                int colDepth = addedValsRow[j];
                if (colDepth >= currentDepth)
                {
                    // coordinate pair 1 is at the top left
                    if (xpos1 == -1)
                    {
                        xpos1 = j;
                        ypos1 = i - (colDepth - 1);
                    }
                    // coordinate pair 2 is at the bottom right
                    xpos2 = j;
                    //ypos2 = i

                    // limit depths that are bigger than the one that's being checked
                    if (colDepth > currentDepth)
                    {
                        area += currentDepth;
                    }
                    else
                    {
                        area += colDepth;
                    }
                }
            }

            if (area > maxArea)
            {
                maxArea = area;
                maxXpos1 = xpos1;
                maxYpos1 = ypos1;
                maxXpos2 = xpos2;
                maxYpos2 = ypos2;
            }
        }

        free(uniqueDepths);
    }

    free(addedValsRow);

    int* corners = (int*)malloc(4*sizeof(int));
    corners[0] = maxXpos1;
    corners[1] = maxYpos1;
    corners[2] = maxXpos2;
    corners[3] = maxYpos2;

    return corners;
}

std::vector<bool> VideoTranscoder::compressFrame(CharInfo* currentFrame, CharInfo* prevFrame)
{
    const bool prevFrameExists = (prevFrame != nullptr);
    const uint32_t arraySize = vidTWidth * vidTHeight; 
    std::vector<bool> result;

    std::map<ulong, bool*> bitmaps;

    // making bitmaps of all CharInfos
    for (uint32_t index = 0; index < arraySize; index++)
    {
        char* ciBytes = BinaryUtils::charInfoToCharArray(currentFrame[index]);
        ulong currentCIHash = BinaryUtils::charArrayToUint(ciBytes, sizeof(CharInfo));
        free(ciBytes);

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
            if (bitmaps.count(currentCIHash) == 0)
            {
                bool* binMat = (bool*)malloc(arraySize*sizeof(bool));
                for (int i = 0; i < arraySize; i++)
                {
                    binMat[i] = 0;
                }
                
                bitmaps.insert({currentCIHash, binMat});
            }

            // mark occurence
            bitmaps[currentCIHash][index] = 1;
        }
    }

    bool newInfo = bitmaps.size() > 0;
    if (!newInfo)
    {
        result.push_back(1);    // marks a frame being the same as the previous one
    }
    else
    {
        result.push_back(0);    // marks new info

        // now compress the bitmaps
        for (std::map<ulong, bool*>::iterator it = bitmaps.begin(); it != bitmaps.end(); it++)
        {
            ulong ciHash = it->first;
            bool* bitmap = it->second;
            // append CI bits
            char* ciHashBytes = BinaryUtils::numToCharArray(ciHash);
            for (int i = 1; i <= sizeof(CharInfo); i++) // start at the second byte, since CIs only have 7 but ulongs have 8
            {
                char c = ciHashBytes[i];
                for (int j = 0; j < 8; j++)
                {
                    bool bit = (bool)((c >> (7-j)) & 1);
                    result.push_back(bit);
                }
            }
            free(ciHashBytes);

            int* rect = findBiggestRectangle(bitmap, arraySize*sizeof(bool), vidTWidth);
            while(rect[0] != -1)
            {
                // write rectangle info to resulting bit vector
                // true if the rectangle is just 1 element
                if (rect[0] == rect[2] && rect[1] == rect[3])
                {
                    // 0b01 is the code for position
                    result.push_back(0);
                    result.push_back(1);

                    for (int i = 0; i < 2; i++)
                    {
                        char* numBytes = BinaryUtils::numToCharArray((uint16_t) rect[i]);
                        bool* numBits = BinaryUtils::charArrayToBoolArray(numBytes, 2);
                        VariousUtils::pushArrayToVector(numBits, &result, 16);
                        free(numBits);
                        free(numBytes);
                    }
                }
                else
                {
                    // 0b00 is the code for rectangle
                    result.push_back(0);
                    result.push_back(0);

                    for (int i = 0; i < 4; i++)
                    {
                        char* numBytes = BinaryUtils::numToCharArray((uint16_t) rect[i]);
                        bool* numBits = BinaryUtils::charArrayToBoolArray(numBytes, 2);
                        VariousUtils::pushArrayToVector(numBits, &result, 16);
                        free(numBits);
                        free(numBytes);
                    }
                }

                // clear rectangle from bitmap
                // y coordinate
                for (int i = rect[1]; i <= rect[3]; i++)
                {
                    // x coordinate
                    for (int j = rect[0]; j <= rect[2]; j++)
                    {
                        bitmap[i*vidTWidth+j] = 0;
                    }
                }

                free(rect);
                rect = findBiggestRectangle(bitmap, arraySize*sizeof(bool), vidTWidth);
            }
            free(rect);

            // 0b10 is the code for end of CI segment
            result.push_back(1);
            result.push_back(0);

            free(bitmap);
        }

        // replace last end of CI (0b10) with end of frame (0b11)
        result[result.size()-1] = 1;
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
    cv::Vec3b lab1;
    cv::cvtColor(v1, lab1, cv::COLOR_RGB2Lab);
    cv::Vec3b lab2;
    cv::cvtColor(v2, lab2, cv::COLOR_RGB2Lab);

    return pow((lab1[0] / (255./100.)) - (lab2[0] / (255./100.)), 2) + 
           pow((lab1[1] - 128) - (lab2[1] - 128), 2) + 
           pow((lab1[2] - 128) - (lab2[2] - 128), 2);
}

CharInfo findBestBlockCharacter(cv::Mat img)
{
    CharInfo minDiffCharInfo;
    int minDiff = getRGBDiff(cv::Vec3b(0, 0, 0), cv::Vec3b(255, 255, 255)) + 1;
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
                CharInfo best = findBestBlockCharacter(framePart);
                
                for (int i = 0; i < 3; i++)
                {
                    frameInfo[charIndex].foregroundRGB[i] = best.foregroundRGB[i];
                    frameInfo[charIndex].backgroundRGB[i] = best.backgroundRGB[i];
                }
                frameInfo[charIndex].chara = best.chara;
            }
        }
    }
    else
    {
        std::cout << "downscaling\n";
    }

    return frameInfo;
}