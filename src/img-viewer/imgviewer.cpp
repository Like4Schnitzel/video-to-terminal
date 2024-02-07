#include "imgviewer.hpp"

namespace vtt {

ImgViewer::ImgViewer(const std::string path)
{
    file = cv::imread(path);
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
    // loop through the height characters from 1/8th to 7/8ths block
    const double eigthHeight = imageHeight / 8.0;
    double currentHeight = imageHeight;
    // don't need to check full block (8) since it'll just go with fg=bg at check 1
    for (currentOption = 1; currentOption < 8; currentOption++)
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

    // loop through the width characters from 7/8ths to 1/8th
    const double eigthWidth = imageWidth / 8.0;
    double currentWidth = imageWidth;
    for (currentOption = 9; currentOption < 0xF; currentOption++)
    {
        currentWidth -= eigthWidth;

        fgRect = img(cv::Rect(0, 0, round(currentWidth), imageHeight));
        if (fgRect.rows == 0) continue;
        cv::Scalar avrgFgBGR = cv::mean(fgRect);

        bgRect = img(cv::Rect(round(currentWidth), 0, imageWidth - round(currentWidth), imageHeight));
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
        }
    }

    // corner characters. They're weird.
    // we're saving each of the means so that we can use them for calculations later
    const int halfWidth = imageWidth / 2;
    const int halfHeight = imageHeight / 2;
    if (halfHeight > 0 && halfWidth > 0)
    {
        const int cornerArea = halfHeight * halfWidth;

        fgRect = img(cv::Rect(0, halfHeight, halfWidth, halfHeight));
        cv::Scalar lowLeftMean = cv::mean(fgRect);

        fgRect = img(cv::Rect(halfWidth, halfHeight, halfWidth, halfHeight));
        cv::Scalar lowRightMean = cv::mean(fgRect);

        fgRect = img(cv::Rect(0, 0, halfWidth, halfHeight));
        cv::Scalar upLeftMean = cv::mean(fgRect);

        fgRect = img(cv::Rect(halfWidth, 0, halfWidth, halfHeight));
        cv::Scalar upRightMean = cv::mean(fgRect);

        cv::Scalar fgMean;
        cv::Scalar bgMean;

        const auto checkDiff = [&maxDiff, &maxDiffCharInfo, &fgMean, &bgMean, &currentOption]()
        {
            int colorDiff = getColorDiff(fgMean, bgMean);
            if (colorDiff > maxDiff)
            {
                maxDiff = colorDiff;
                for (int i = 0; i < 3; i++)
                {
                    // turn around BGR so RGB gets saved instead
                    maxDiffCharInfo.foregroundRGB[i] = fgMean[2-i];
                    maxDiffCharInfo.backgroundRGB[i] = bgMean[2-i];
                }
                maxDiffCharInfo.chara = currentOption;
            }
        };

        currentOption = 0x16;
        fgMean = lowLeftMean;
        bgMean = (lowRightMean + upLeftMean + upRightMean) / 3;
        checkDiff();

        currentOption = 0x17;
        fgMean = lowRightMean;
        bgMean = (lowLeftMean + upLeftMean + upRightMean) / 3;
        checkDiff();

        currentOption = 0x18;
        fgMean = upLeftMean;
        bgMean = (lowLeftMean + lowRightMean + upRightMean) / 3;
        checkDiff();

        // I don't fucking know why it just skips 4 elements until it does the last corner block but that's how it is.
        currentOption = 0x1D;
        fgMean = upRightMean;
        bgMean = (lowLeftMean + lowRightMean + upLeftMean) / 3;
        checkDiff();

        // And now the double corner character.
        currentOption = 0x1A;
        fgMean = (upLeftMean + lowRightMean) / 2;
        bgMean = (lowLeftMean + upRightMean) / 2;
        checkDiff();
    }

    return maxDiffCharInfo;
}

void ImgViewer::transcode(const int width, const int height)
{
    this->width = width;
    this->height = height;
    transcodedFile.resize(width*height);

    const int widthPixelsPerChar = file.size().width / this->width;
    const int heightPixelsPerChar = file.size().height / this->height;

    int charIndex = 0;
    std::vector<std::thread> threads;
    threads.reserve(this->height);
    for (int i = 0; i < this->height; i++)
    {
        threads.emplace_back([&, charIndex, i](){
            int ciIndexCopy = charIndex;
            int y = heightPixelsPerChar * i;
            for (int j = 0; j < this->width; j++)
            {
                int x = widthPixelsPerChar * j;
                CharInfo best = findBestBlockCharacter(
                    file(cv::Rect((int)x, (int)y, (int)widthPixelsPerChar, (int)heightPixelsPerChar))
                );

                for (int k = 0; k < 3; k++)
                {
                    transcodedFile[ciIndexCopy].foregroundRGB[k] = best.foregroundRGB[k];
                    transcodedFile[ciIndexCopy].backgroundRGB[k] = best.backgroundRGB[k];
                }
                transcodedFile[ciIndexCopy].chara = best.chara;

                ciIndexCopy++;
            }
        });
        charIndex += this->width;
    }

    // wait for all threads to finish
    for (auto& thread : threads)
    {
        thread.join();
    }
}

void ImgViewer::print()
{
    if (transcodedFile.size() == 0) throw std::logic_error("File has not been transcoded yet.");

    std::cout << "\x1B[2J\x1B[H";
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            auto ciAtLocation = transcodedFile[i*width+j];
            std::string fgColorSetter = "\x1B[38;2";
            std::string bgColorSetter = "\x1B[48;2";
            for (int k = 0; k < 3; k++)
            {
                fgColorSetter += ";" + std::to_string((int)ciAtLocation.foregroundRGB[k]);
                bgColorSetter += ";" + std::to_string((int)ciAtLocation.backgroundRGB[k]);
            }
            fgColorSetter += "m";
            bgColorSetter += "m";

            std::cout << fgColorSetter << bgColorSetter << VariousUtils::numToUnicodeBlockChar(ciAtLocation.chara);
        }
        std::cout << "\x1B[0m\n";
    }
}

}
