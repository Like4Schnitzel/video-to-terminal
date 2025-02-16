#include "videotranscoder.hpp"

namespace vtt {

VideoTranscoder::VideoTranscoder(const std::string path, const std::string vtdiPath, const uint16_t terminalWidth, const uint16_t terminalHeight)
{
    vidPath = path;
    this->vtdiPath = vtdiPath;
    vidCap.open(vidPath);
    if (!vidCap.isOpened())
    {
        throw std::invalid_argument("The video at the provided path could not be read.");
    }

    vidWidth = vidCap.get(cv::CAP_PROP_FRAME_WIDTH);
    vidHeight = vidCap.get(cv::CAP_PROP_FRAME_HEIGHT);
    vidFPS = vidCap.get(cv::CAP_PROP_FPS);
    vidFrames = vidCap.get(cv::CAP_PROP_FRAME_COUNT);
    vidTWidth = terminalWidth;
    vidTHeight = terminalHeight;

    widthPixelsPerChar = (double) vidWidth / vidTWidth;
    heightPixelsPerChar = (double) vidHeight / vidTHeight;
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

void VideoTranscoder::transcodeFile(const uint maxThreads)
{
    constexpr uint16_t versionNumber = 2;   // change if updates to the file format are made

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

    // setting constants for video byte writing
    const int totalTerminalChars = vidTWidth * vidTHeight;
    // plus 1 to keep one previous frame saved in transcodedFrames[0]
    std::unique_ptr<std::shared_ptr<CharInfo[]>[]> transcodedFrames = std::make_unique<std::shared_ptr<CharInfo[]>[]>(maxThreads+1);
    std::unique_ptr<std::thread[]> transcodingThreads = std::make_unique<std::thread[]>(maxThreads);
    std::unique_ptr<std::thread[]> compressionThreads = std::make_unique<std::thread[]>(maxThreads);
    for (int i = 1; i <= maxThreads; i++)
    {
        transcodedFrames[i] = std::make_shared<CharInfo[]>(totalTerminalChars);
    }

    uint32_t frameBytesIndex = 0;
    int completedFrames = 0;
    double previousProgress = -1;

    const auto updateProgress = [&completedFrames, &previousProgress, *this]()
    {
        double newProgress = (int)((double) completedFrames / this->vidFrames * 10000) / 100.;  // round to 4 digits
        if (newProgress != previousProgress)
        {
            previousProgress = newProgress;
            std::cout << std::fixed << std::setprecision(2) << "\33[2K\r" << newProgress << "\% done..." << std::flush;
        }
    };

    const auto transcodeOneFrame = [&transcodedFrames, &completedFrames, updateProgress, *this](int i, cv::Mat img)
    {
        imgToCIMat(img, this->vidTWidth, this->vidTHeight, transcodedFrames[i]);
        completedFrames++;
        updateProgress();
    };

    std::cout << "Transcoding frames...\n";

    do
    {
        int i;
        for (i = 0; i < maxThreads; i++)
        {
            vidCap>>frame;
            if (frame.empty())
                break;
            transcodingThreads[i] = std::thread(transcodeOneFrame, i+1, frame.clone());
            compressionThreads[i] = std::thread([&transcodingThreads, &compressionThreads, &transcodedFrames, this, i](){
                transcodingThreads[i].join();
                if (i > 0) compressionThreads[i-1].join();
                std::vector<Byte> frameBytes = compressFrame(transcodedFrames[i], transcodedFrames[i-1]);
                BinaryUtils::writeToFile(vtdiPath, (char*)frameBytes.data(), frameBytes.size(), true);
            });
        }

        compressionThreads[maxThreads-1].join();

        transcodedFrames[0] = transcodedFrames[maxThreads];
        transcodedFrames[maxThreads] = std::make_shared<CharInfo[]>(totalTerminalChars);
    } while (!frame.empty());

    std::cout << "\33[2k\r" << "100\% done!     \n" << std::flush;
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
                addedValsRow[j]++;
                // if addedValsRow[j] not already in uniqueDepths, add it
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

        // say there are 2 separated shapes of 1s in the bitmap that are separated by at least one line of 0s
        // we don't need to check both in the same call since we'll go through both of them either way
        // this does technically make the name of the function inaccurate since it just finds the biggest rectangle in the first shape now
        // but in the context of this program it'll do its job faster than before
        if (uniqueDepths.size() == 0 && maxPositions[0] != -1)
            return maxPositions;

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
                // break early if there's a horizontally disconnected shape
                else if (colDepth == 0 && attemptPositions[0] != -1)
                    break;
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
        result.push_back(0);    // marks new info

        // now compress the bitmaps
        for (auto it = bitmaps.begin(); it != bitmaps.end(); it++)
        {
            ulong ciHash = it->first;
            auto bitmap = it->second;

            // append CI bits
            auto ciHashBytes = BinaryUtils::numToByteArray(ciHash);
            for (int i = 1; i <= sizeof(CharInfo); i++) // start at the second byte, since CIs only have 7 but ulongs have 8
            {
                result.push_back(ciHashBytes[i]);
            }

            auto rect = findBiggestRectangle(bitmap, arraySize*sizeof(bool), vidTWidth);
            while(rect[0] != -1)
            {
                // write rectangle info to resulting bit vector
                // true if the rectangle is just 1 element
                if (rect[0] == rect[2] && rect[1] == rect[3])
                {
                    // 1 is the code for position
                    result.push_back(1);

                    for (int i = 0; i < 2; i++)
                    {
                        for (Byte byte : BinaryUtils::numToByteArray((uint16_t) rect[i]))
                        {
                            result.push_back(byte);
                        }
                    }
                }
                else
                {
                    // 0 is the code for rectangle
                    result.push_back(0);

                    for (int i = 0; i < 4; i++)
                    {
                        for (Byte byte : BinaryUtils::numToByteArray((uint16_t) rect[i]))
                        {
                            result.push_back(byte);
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
            result.push_back(2);
        }

        // replace last end of CI (2) with end of frame (3)
        result[result.size()-1] = 3;
    }

    return result;
}

}
