#include "VideoTranscoder.hpp"

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
    const uint16_t versionNumber = 2;   // change if updates to the file format are made

    // reset output file just in case
    BinaryUtils::writeToFile(vtdiPath, (char*)nullptr, 0, false);
    // write all the pre-video information to the file
    auto args = std::make_tuple(
        uint8_t(86), uint8_t(84), uint8_t(68), uint8_t(73), versionNumber, vidFrames, vidFPS, vidTWidth, vidTHeight
    );
    std::apply([&](auto... args)
    {
        (..., BinaryUtils::writeToFile(vtdiPath, (char*)BinaryUtils::numToByteArray(args).data(), sizeof(args), true));
    }, args);

    // settings constants for video byte writing
    const int totalTerminalChars = vidTWidth * vidTHeight;
    std::shared_ptr<CharInfo[]> frameCIs = std::make_shared<CharInfo[]>(totalTerminalChars);
    std::shared_ptr<CharInfo[]> previousFrameChars;

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

        frameCIs = transcodeFrame();
        std::vector<Byte> frameBytes = compressFrame(frameCIs, previousFrameChars);
        BinaryUtils::writeToFile(vtdiPath, (char*)frameBytes.data(), frameBytes.size(), true);
        previousFrameChars = frameCIs;
    }

    std::cout << "\33[2k\r" << "100\% done!    \n" << std::flush;
}

auto findBiggestRectangle(const std::shared_ptr<bool[]> bitmap, const int bitCount, const int rowLength)
{
    int maxArea = 0;
    // -1 will be returned if nothing has been found
    std::array<int, 4> maxPositions; // coordinate pairs for the corners of the rectangle
    maxPositions[0] = -1;

    std::vector<int> addedValsRow(rowLength, 0);

    std::vector<int> uniqueDepths;
    uniqueDepths.reserve(rowLength);
    const int rows = bitCount / rowLength;
    for (int i = 0; i < rows; i++)
    {
        uniqueDepths.clear();

        // overlay values with above rows
        for (int j = 0; j < rowLength; j++)
        {
            if (bitmap.get()[i*rowLength+j] == 0)
            {
                addedValsRow[j] = 0;
            }
            else
            {
                addedValsRow[j] = addedValsRow[j]+1;
                if (std::none_of(
                    uniqueDepths.begin(),
                    uniqueDepths.end(),
                    [v = addedValsRow[j]](auto d){return d == v;}
                ))
                {
                    uniqueDepths.push_back(addedValsRow[j]);
                }
            }
        }

        // find biggest rectangle
        for (auto& currentDepth : uniqueDepths)
        {
            int area = 0;
            std::array<int, 4> attemptPositions = {-1, 0, 0, i};

            for (int j = 0; j < rowLength; j++)
            {
                int colDepth = addedValsRow[j];
                if (colDepth >= currentDepth)
                {
                    // coordinate pair 1 is at the top left
                    if (attemptPositions[0] == -1)
                    {
                        attemptPositions[0] = j;
                        attemptPositions[1] = i - (colDepth - 1);
                    }
                    // coordinate pair 2 is at the bottom right
                    attemptPositions[2] = j;
                    //attemptPositions[3] = i

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
                // dw, this is a deep copy
                maxPositions = attemptPositions;
            }
        }
    }

    return maxPositions;
}

std::vector<Byte> VideoTranscoder::compressFrame(std::shared_ptr<CharInfo[]> currentFrame, std::shared_ptr<CharInfo[]> prevFrame)
{
    const bool prevFrameExists = (prevFrame.get() != nullptr);
    const uint32_t arraySize = vidTWidth * vidTHeight; 
    std::vector<Byte> result;

    std::map<ulong, std::shared_ptr<bool[]>> bitmaps;

    // making bitmaps of all CharInfos
    for (uint32_t index = 0; index < arraySize; index++)
    {
        ulong currentCIHash = BinaryUtils::byteArrayToUint(
            BinaryUtils::charInfoToByteArray(currentFrame[index]).data(),
            sizeof(CharInfo)
        );

        ulong prevCIHash;
        if (prevFrameExists)
        {
            prevCIHash = BinaryUtils::byteArrayToUint(
                BinaryUtils::charInfoToByteArray(prevFrame[index]).data(),
                sizeof(CharInfo)
            );
        }

        // only proceed if the CI has changed from the last frame
        if (!prevFrameExists || currentCIHash != prevCIHash)
        {
            // if new charinfo, create bitmap
            if (bitmaps.count(currentCIHash) == 0)
            {
                std::shared_ptr<bool[]> binMat = std::make_shared<bool[]>(arraySize);
                for (int i = 0; i < arraySize; i++)
                {
                    binMat.get()[i] = 0;
                }
                
                bitmaps.insert({currentCIHash, binMat});
            }

            // mark occurence
            bitmaps[currentCIHash].get()[index] = 1;
        }
    }

    bool newInfo = bitmaps.size() > 0;
    if (!newInfo)
    {
        result.push_back(1);    // marks a frame being the same as the previous one
    }
    else
    {
        std::vector<std::thread> threads;
        std::vector<std::vector<Byte>> threadResults;
        threads.reserve(bitmaps.size());
        threadResults.resize(bitmaps.size());

        result.push_back(0);    // marks new info

        // now compress the bitmaps
        for (auto it = bitmaps.begin(); it != bitmaps.end(); it++)
        {
            ulong ciHash = it->first;
            auto bitmap = it->second;

            threads.emplace_back(
            [
                &compressedBytes = threadResults[threads.size()],
                ciHash,
                bitmap,
                vidTWidth = this->vidTWidth,
                arraySize
            ]
            ()
            {
                // append CI bits
                auto ciHashBytes = BinaryUtils::numToByteArray(ciHash);
                for (int i = 1; i <= sizeof(CharInfo); i++) // start at the second byte, since CIs only have 7 but ulongs have 8
                {
                    compressedBytes.push_back(ciHashBytes[i]);
                }

                auto rect = findBiggestRectangle(bitmap, arraySize*sizeof(bool), vidTWidth);
                while(rect[0] != -1)
                {
                    // write rectangle info to resulting bit vector
                    // true if the rectangle is just 1 element
                    if (rect[0] == rect[2] && rect[1] == rect[3])
                    {
                        // 1 is the code for position
                        compressedBytes.push_back(1);

                        for (int i = 0; i < 2; i++)
                        {
                            for (Byte byte : BinaryUtils::numToByteArray((uint16_t) rect[i]))
                            {
                                compressedBytes.push_back(byte);
                            }
                        }
                    }
                    else
                    {
                        // 0 is the code for rectangle
                        compressedBytes.push_back(0);

                        for (int i = 0; i < 4; i++)
                        {
                            for (Byte byte : BinaryUtils::numToByteArray((uint16_t) rect[i]))
                            {
                                compressedBytes.push_back(byte);
                            }
                        }
                    }

                    // clear rectangle from bitmap
                    // y coordinate
                    for (int i = rect[1]; i <= rect[3]; i++)
                    {
                        // x coordinate
                        for (int j = rect[0]; j <= rect[2]; j++)
                        {
                            bitmap.get()[i*vidTWidth+j] = 0;
                        }
                    }

                    rect = findBiggestRectangle(bitmap, arraySize*sizeof(bool), vidTWidth);
                }

                // 2 is the code for end of CI segment
                compressedBytes.push_back(2);
            });
        }

        // wait for all threads to finish
        for (int i = 0; i < threads.size(); i++)
        {
            threads[i].join();
            for (Byte byte : threadResults[i])
            {
                result.push_back(byte);
            }
        }

        // replace last end of CI (2) with end of frame (3)
        result[result.size()-1] = 3;
    }

    return result;
}

int getColorDiff(cv::Scalar col1, cv::Scalar col2)
{
    return pow(col1[0] - col2[0], 2) + 
           pow(col1[1] - col2[1], 2) + 
           pow(col1[2] - col2[2], 2);
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
    // don't need to check full block (8) since it'll just go with fg=bg at check 1
    for (currentOption = 1; currentOption < 7; currentOption++)
    {
        currentHeight -= eigthHeight;

        fgRect = img(cv::Rect(0, round(currentHeight), imageWidth, imageHeight-round(currentHeight)));
        if (fgRect.rows == 0) continue;
        cv::Scalar avrgFgBGR = cv::mean(fgRect);

        bgRect = img(cv::Rect(0, 0, imageWidth, round(currentHeight)));
        if (bgRect.rows == 0) continue;
        cv::Scalar avrgBgBGR = cv::mean(bgRect);

        int colorDiff = getColorDiff(avrgFgBGR, avrgBgBGR);    
        if (colorDiff > maxDiff)
        {
            maxDiff = colorDiff;
            for (int i = 0; i < 3; i++)
            {
                // turn around BGR so RGB gets saved instead
                maxDiffCharInfo.foregroundRGB[i] = avrgFgBGR[2-i];
                maxDiffCharInfo.backgroundRGB[i] = avrgBgBGR[2-i];
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

std::shared_ptr<CharInfo []> VideoTranscoder::transcodeFrame()
{
    std::shared_ptr<CharInfo[]> frameInfo = std::make_shared<CharInfo[]>(vidTWidth*vidTHeight);
    int charIndex = 0;

    const int widthPixelsPerChar = vidWidth / vidTWidth;
    const int heightPixelsPerChar = vidHeight / vidTHeight;

    std::vector<std::thread> threads;
    threads.reserve(vidTHeight);
    for (int i = 0; i < vidTHeight; i++)
    {
        threads.emplace_back([&, charIndex, i](){
            int ciIndexCopy = charIndex;
            int y = heightPixelsPerChar * i;
            for (int j = 0; j < vidTWidth; j++)
            {
                int x = widthPixelsPerChar * j;
                CharInfo best = findBestBlockCharacter(
                    frame(cv::Rect((int)x, (int)y, (int)widthPixelsPerChar, (int)heightPixelsPerChar))
                );

                for (int k = 0; k < 3; k++)
                {
                    frameInfo.get()[ciIndexCopy].foregroundRGB[k] = best.foregroundRGB[k];
                    frameInfo.get()[ciIndexCopy].backgroundRGB[k] = best.backgroundRGB[k];
                }
                frameInfo.get()[ciIndexCopy].chara = best.chara;

                ciIndexCopy++;
            }
        });
        charIndex += vidTWidth;
    }

    // wait for all threads to finish
    for (auto& thread : threads)
    {
        thread.join();
    }

    return frameInfo;
}
