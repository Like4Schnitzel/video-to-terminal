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
        (..., BinaryUtils::writeToFile(vtdiFilePath, (char*)BinaryUtils::numToByteArray(args), sizeof(args), true, true));
    }, args);

    // settings constants for video byte writing
    const int totalTerminalChars = vidTWidth * vidTHeight;
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
        Byte* frameBytes = BinaryUtils::bitArrayToByteArray(frameBits, frameBits.size());
        BinaryUtils::writeToFile(vtdiFilePath, (char*)frameBytes, frameBits.size()/8, true);
        free(frameBytes);

        free(previousFrameChars);
        previousFrameChars = frameChars;
    }
    free(previousFrameChars);

    std::cout << "\33[2k\r" << "100\% done!    \n" << std::flush;
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
        Byte* ciBytes = BinaryUtils::charInfoToByteArray(currentFrame[index]);
        ulong currentCIHash = BinaryUtils::byteArrayToUint(ciBytes, sizeof(CharInfo));
        free(ciBytes);

        ulong prevCIHash;
        if (prevFrameExists)
        {
            ciBytes = BinaryUtils::charInfoToByteArray(prevFrame[index]);
            prevCIHash = BinaryUtils::byteArrayToUint(ciBytes, sizeof(CharInfo));
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
            Byte* ciHashBytes = BinaryUtils::numToByteArray(ciHash);
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
                        Byte* numBytes = BinaryUtils::numToByteArray((uint16_t) rect[i]);
                        bool* numBits = BinaryUtils::byteArrayToBitArray(numBytes, 2);
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
                        Byte* numBytes = BinaryUtils::numToByteArray((uint16_t) rect[i]);
                        bool* numBits = BinaryUtils::byteArrayToBitArray(numBytes, 2);
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

int getColorDiff(cv::Mat dom1, cv::Mat dom2)
{
    return pow((dom1.data[0] / (255./100.)) - (dom2.data[0] / (255./100.)), 2) + 
           pow((dom1.data[1] - 128) - (dom2.data[1] - 128), 2) + 
           pow((dom1.data[2] - 128) - (dom2.data[2] - 128), 2);
}

CharInfo findBestBlockCharacter(cv::Mat img)
{
    CharInfo maxDiffCharInfo;
    int maxDiff = -1;
    const int imageHeight = img.size().height;
    const int imageWidth = img.size().width;
    int currentOption;
    cv::Mat fgBGR;
    cv::Mat bgBGR;
    cv::Mat fgBGRFlt;
    cv::Mat bgBGRFlt;
    cv::Mat fgDomBGRClr;
    cv::Mat bgDomBGRClr;
    cv::Mat fgLab;
    cv::Mat bgLab;
    cv::Mat fgLabFlt;
    cv::Mat bgLabFlt;
    cv::Mat fgDomLabClr;
    cv::Mat bgDomLabClr;
    cv::TermCriteria criteria = cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 10, 1.0);
    cv::Mat indices;

    // skip upper half (0) since lower half can be used
    // loop through the lower eights
    const double eigthHeight = (double)imageHeight / 8;
    double currentHeight = imageHeight;
    for (currentOption = 1; currentOption < 8; currentOption++)
    {
        currentHeight -= eigthHeight;
        
        fgBGR = img(cv::Rect(0, (int)currentHeight, imageWidth, imageHeight-(int)currentHeight));
        cv::cvtColor(fgBGR, fgLab, cv::COLOR_BGR2Lab);
        fgLab.convertTo(fgLabFlt, CV_32FC1);
        cv::kmeans(fgLabFlt, 1, indices, criteria, 10, cv::KMEANS_RANDOM_CENTERS, fgDomLabClr);

        bgBGR = img(cv::Rect(0, 0, imageWidth, imageHeight-(int)currentHeight));
        cv::cvtColor(bgBGR, bgLab, cv::COLOR_BGR2Lab);
        bgLab.convertTo(bgLabFlt, CV_32FC1);
        cv::kmeans(bgLabFlt, 1, indices, criteria, 10, cv::KMEANS_RANDOM_CENTERS, bgDomLabClr);

        int colorDiff = getColorDiff(fgDomLabClr, bgDomLabClr);
        if (maxDiff == -1 || colorDiff > maxDiff)
        {
            // Lab is great for detecting human perceivable difference in color
            // but for video displaying in the terminal I need RGB values
            fgBGR.convertTo(fgBGRFlt, CV_32FC1);
            bgBGR.convertTo(bgBGRFlt, CV_32FC1);
            cv::kmeans(fgBGRFlt, 1, indices, criteria, 10, cv::KMEANS_RANDOM_CENTERS, fgDomBGRClr);
            cv::kmeans(bgBGRFlt, 1, indices, criteria, 10, cv::KMEANS_RANDOM_CENTERS, bgDomBGRClr);

            maxDiff = colorDiff;
            for (int i = 0; i < 3; i++)
            {
                // turn around BGR so RGB gets saved instead
                maxDiffCharInfo.foregroundRGB[i] = fgDomBGRClr.data[2-i];
                maxDiffCharInfo.backgroundRGB[i] = bgDomBGRClr.data[2-i];
            }
            maxDiffCharInfo.chara = currentOption;
        }
    }

    return maxDiffCharInfo;
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
        for (double y = 0; y < vidHeight; y += heightPixelsPerChar)
        {
            if (y + heightPixelsPerChar > vidHeight) y = vidHeight - heightPixelsPerChar;
            for (double x = 0; x < vidWidth; x += widthPixelsPerChar)
            {
                if (x + widthPixelsPerChar > vidWidth) x = vidWidth - widthPixelsPerChar;
                cv::Mat framePart = this->frame(cv::Rect((int)x, (int)y, (int)widthPixelsPerChar, (int)heightPixelsPerChar));
                CharInfo best = findBestBlockCharacter(framePart);

                for (int i = 0; i < 3; i++)
                {
                    frameInfo[charIndex].foregroundRGB[i] = best.foregroundRGB[i];
                    frameInfo[charIndex].backgroundRGB[i] = best.backgroundRGB[i];
                }
                frameInfo[charIndex].chara = best.chara;

                charIndex++;
            }
        }
    }
    else
    {
        std::cout << "downscaling\n";
    }

    return frameInfo;
}