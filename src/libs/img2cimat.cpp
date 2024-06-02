#include "img2cimat.hpp"

namespace vtt {

std::vector<CharInfo> imgToCIMat(const cv::Mat& img, const uint width, const uint height)
{
    std::vector<CharInfo> result;
    result.resize(width*height);

    imgToCIMat(img, width, height, result);

    return result;
}

cv::Vec3b meanColWithSubPixels(cv::Mat mat, double x, double y, double width, double height)
{
    // thanks to https://github.com/marceldobehere/ for helping me out with this one
    cv::Vec3d result = {0, 0, 0};
    double pixelCount = 0;

    int x0 = (int)floor(x);
    int y0 = (int)floor(y);
    int x1 = (int)ceil(x+width);
    int y1 = (int)ceil(y+height);

    double ax0 = x;
    double ay0 = y;
    double ax1 = x + width;
    double ay1 = y + height;

    double x0Percent = ax0 - x0;
    double y0Percent = ay0 - y0;
    double x1Percent = x1 - ax1;
    double y1Percent = y1 - ay1;

    int maxW = min(x1 - x0 + 1, mat.cols - x0 - 1);
    int maxH = min(y1 - y0 + 1, mat.rows - y0 - 1);
    auto slice = mat(cv::Rect(x0, y0, maxW, maxH));

    for (int y = 0; y < maxH; y++)
        for (int x = 0; x < maxW; x++)
        {
            double pixelPercent = 1.0;
            if (x == 0)
                pixelPercent *= x0Percent;
            if (x == maxW - 1)
                pixelPercent *= x1Percent;
            if (y == 0)
                pixelPercent *= y0Percent;
            if (y == maxH - 1)
                pixelPercent *= y1Percent;
            
            auto pixel = slice.at<cv::Vec3b>(y, x);
            result += pixel * pixelPercent;
            pixelCount += pixelPercent;
        }
        
        
        
    result /= pixelCount;
    return (cv::Vec3b)result;
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

int getColorDiff(cv::Scalar col1, cv::Scalar col2)
{
    return pow(col1[0] - col2[0], 2) + 
           pow(col1[1] - col2[1], 2) + 
           pow(col1[2] - col2[2], 2);
}

double min(double a, double b) {if (a < b) return a; else return b;}
double max(double a, double b) {if (a < b) return b; else return a;}

}
