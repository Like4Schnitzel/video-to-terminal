#include "VideoTranscoder.hpp"
#include "BinaryUtils.hpp"
#include "VariousUtils.hpp"

VideoTranscoder::VideoTranscoder(const std::string path, const std::string vtdiPath, const uint16_t terminalWidth, const uint16_t terminalHeight)
{
    vidPath = path;
    this->vtdiPath = vtdiPath;
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
    const uint16_t versionNumber = 1;   // change if updates to the file format are made

    // reset output file just in case
    BinaryUtils::writeToFile(vtdiPath, (char*)nullptr, 0, false);
    // write all the pre-video information to the file
    auto args = std::make_tuple(
        uint8_t(86), uint8_t(84), uint8_t(68), uint8_t(73), versionNumber, vidFrames, vidFPS, vidTWidth, vidTHeight
    );
    std::apply([&](auto... args)
    {
        (..., BinaryUtils::writeToFile(vtdiPath, (char*)BinaryUtils::numToByteArray(args), sizeof(args), true, true));
    }, args);

    // settings constants for video byte writing
    const int totalTerminalChars = vidTWidth * vidTHeight;
    CharInfo* frameChars;
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

        frameChars = transcodeFrame();
        std::vector<bool> frameBits = compressFrame(frameChars, previousFrameChars);
        // pad bits to full byte
        while (frameBits.size() % 8 != 0)
        {
            frameBits.push_back(0);
        }
        Byte* frameBytes = BinaryUtils::bitArrayToByteArray(frameBits, frameBits.size());
        BinaryUtils::writeToFile(vtdiPath, (char*)frameBytes, frameBits.size()/8, true);
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
        auto compressCI = [](std::vector<bool>& compressedBits, ulong ciHash, bool* bitmap, int vidTWidth, int arraySize)
        {
            // append CI bits
            Byte* ciHashBytes = BinaryUtils::numToByteArray(ciHash);
            for (int i = 1; i <= sizeof(CharInfo); i++) // start at the second byte, since CIs only have 7 but ulongs have 8
            {
                char c = ciHashBytes[i];
                for (int j = 0; j < 8; j++)
                {
                    bool bit = (bool)((c >> (7-j)) & 1);
                    compressedBits.push_back(bit);
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
                    compressedBits.push_back(0);
                    compressedBits.push_back(1);

                    for (int i = 0; i < 2; i++)
                    {
                        Byte* numBytes = BinaryUtils::numToByteArray((uint16_t) rect[i]);
                        bool* numBits = BinaryUtils::byteArrayToBitArray(numBytes, 2);
                        VariousUtils::pushArrayToVector(numBits, &compressedBits, 16);
                        free(numBits);
                        free(numBytes);
                    }
                }
                else
                {
                    // 0b00 is the code for rectangle
                    compressedBits.push_back(0);
                    compressedBits.push_back(0);

                    for (int i = 0; i < 4; i++)
                    {
                        Byte* numBytes = BinaryUtils::numToByteArray((uint16_t) rect[i]);
                        bool* numBits = BinaryUtils::byteArrayToBitArray(numBytes, 2);
                        VariousUtils::pushArrayToVector(numBits, &compressedBits, 16);
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
            compressedBits.push_back(1);
            compressedBits.push_back(0);

            free(bitmap);
        };

        std::vector<std::thread> threads;
        std::vector<bool>* threadResults = new std::vector<bool>[bitmaps.size()];

        result.push_back(0);    // marks new info

        // now compress the bitmaps
        for (std::map<ulong, bool*>::iterator it = bitmaps.begin(); it != bitmaps.end(); it++)
        {
            ulong ciHash = it->first;
            bool* bitmap = it->second;

            threads.emplace_back(std::thread(compressCI, std::ref(threadResults[threads.size()]), ciHash, bitmap, vidTWidth, arraySize));
        }

        // wait for all threads to finish
        for (int i = 0; i < threads.size(); i++)
        {
            threads[i].join();
            for (bool bit : threadResults[i])
            {
                result.push_back(bit);
            }
        }

        delete[] threadResults;

        // replace last end of CI (0b10) with end of frame (0b11)
        result[result.size()-1] = 1;
    }

    return result;
}

int getColorDiff(cv::Vec3b col1, cv::Vec3b col2)
{
    return pow(col1[0] - col2[0], 2) + 
           pow(col1[1] - col2[1], 2) + 
           pow(col1[2] - col2[2], 2);
}

cv::Vec3b getAverageColor(cv::Mat img)
{
    long sums[3] = {0, 0, 0};

    for (int i = 0; i < img.cols; i++)
    {
        for (int j = 0; j < img.rows; j++)
        {
            cv::Vec3b bgrVals = img.at<cv::Vec3b>(cv::Point(i, j));

            for (int k = 0; k < 3; k++)
            {
                sums[k] += bgrVals[k];
            }
        }
    }

    // turn bgr into rgb
    long temp = sums[0];
    sums[0] = sums[2];
    sums[2] = temp;

    const int totalPixels = img.cols*img.rows;
    for (int i = 0; i < 3; i++)
    {
        sums[i] /= totalPixels;
    }
    
    cv::Vec3b averages = {(uchar)sums[0], (uchar)sums[1], (uchar)sums[2]};

    return averages;
}

CharInfo findBestBlockCharacter(cv::Mat img)
{
    CharInfo maxDiffCharInfo;
    int maxDiff = -1;
    const int imageHeight = img.size().height;
    const int imageWidth = img.size().width;
    int currentOption;
    cv::Mat fgRect, bgRect;

    // skip upper half (0) since lower half can be used
    // loop through the lower eights
    const double eigthHeight = imageHeight / 8.0;
    double currentHeight = imageHeight;
    // don't need to check full block since it'll just go with fg=bg at check 1
    for (currentOption = 1; currentOption < 7; currentOption++)
    {
        currentHeight -= eigthHeight;

        fgRect = img(cv::Rect(0, round(currentHeight), imageWidth, imageHeight-round(currentHeight)));
        cv::Vec3b avrgFgRGB = getAverageColor(fgRect);

        bgRect = img(cv::Rect(0, 0, imageWidth, round(currentHeight)));
        cv::Vec3b avrgBgRGB = getAverageColor(bgRect);

        int colorDiff = getColorDiff(avrgFgRGB, avrgBgRGB);    
        if (colorDiff > maxDiff)
        {
            maxDiff = colorDiff;
            for (int i = 0; i < 3; i++)
            {
                // turn around BGR so RGB gets saved instead
                maxDiffCharInfo.foregroundRGB[i] = avrgFgRGB[i];
                maxDiffCharInfo.backgroundRGB[i] = avrgBgRGB[i];
            }
            maxDiffCharInfo.chara = currentOption;

            // no need to keep checking if the whole img is one color
            if (colorDiff == 0)
            {
                return maxDiffCharInfo;
            }
        }
    }

    return maxDiffCharInfo;
}

CharInfo* VideoTranscoder::transcodeFrame()
{
    auto transcodeRow = [](int vidTWidth, cv::Mat frame, int i, int heightPixelsPerChar, int widthPixelsPerChar, CharInfo* frameInfo, int charIndex)
    {
        int y = heightPixelsPerChar * i;
        for (int j = 0; j < vidTWidth; j++)
        {
            int x = widthPixelsPerChar * j;
            cv::Mat framePart = frame(cv::Rect((int)x, (int)y, (int)widthPixelsPerChar, (int)heightPixelsPerChar));
            CharInfo best = findBestBlockCharacter(framePart);

            for (int k = 0; k < 3; k++)
            {
                frameInfo[charIndex].foregroundRGB[k] = best.foregroundRGB[k];
                frameInfo[charIndex].backgroundRGB[k] = best.backgroundRGB[k];
            }
            frameInfo[charIndex].chara = best.chara;

            charIndex++;
        }
    };

    CharInfo* frameInfo = (CharInfo*)malloc(sizeof(CharInfo) * vidTHeight * vidTWidth);
    int charIndex = 0;

    const int widthPixelsPerChar = vidWidth / vidTWidth;
    const int heightPixelsPerChar = vidHeight / vidTHeight;

    std::vector<std::thread> threads;
    for (int i = 0; i < vidTHeight; i++)
    {
        threads.emplace_back(std::thread(transcodeRow, vidTWidth, frame, i, heightPixelsPerChar, widthPixelsPerChar, frameInfo, charIndex));
        charIndex += vidTWidth;
    }

    // wait for all threads to finish
    for (auto& thread : threads)
    {
        thread.join();
    }

    return frameInfo;
}
