#pragma once

#include <memory>
#include <opencv2/opencv.hpp>
#include "charinfo.hpp"

namespace vtt {

/// @brief Calculates the mean color of a possibly pixel-misaligned submatrix inside a pixel matrix.
cv::Vec3b meanColWithSubPixels(cv::Mat mat, double x, double y, double width, double height);

double min(double a, double b);

double max(double a, double b);

CharInfo findBestBlockCharacter(cv::Mat img);

int getColorDiff(cv::Scalar col1, cv::Scalar col2);

/// @brief Takes an image and transcodes it to a matrix of CharInfos.
/// @param img The image to transcode.
/// @param width The width of the output matrix.
/// @param height The height of the output matrix.
/// @return A vector containing the CharInfo matrix. Saved row-wise with size (width*height).
std::vector<CharInfo> imgToCIMat(const cv::Mat& img, const uint width, const uint height);

/// @brief Takes an image and transcodes it to a matrix of CharInfos.
/// @param img The image to transcode.
/// @param width The width of the output matrix.
/// @param height The height of the output matrix.
/// @param outMat Optionally provided array to use as output variable. Gets overwritten in the process. Make sure it is sized properly.
template <typename T>
void imgToCIMat(const cv::Mat& img, const uint width, const uint height, T& outMat)
{
    constexpr int subPixelMatrixSize = 8;

    const double widthPixelsPerChar = (double) img.size().width / width;
    const double nthWidth = widthPixelsPerChar / subPixelMatrixSize;
    const double heightPixelsPerChar = (double) img.size().height / height;
    const double nthHeight = heightPixelsPerChar / subPixelMatrixSize;

    int charIndex = 0;

    double y = 0;
    for (int i = 0; i < height; i++)
    {
        double x = 0;

        for (int j = 0; j < width; j++)
        {
            double tempY = y;
            // now we're gonna make an 8x8 pixel matrix to pass since we at max have an eighth of a character as resolution
            cv::Mat3b rect(subPixelMatrixSize, subPixelMatrixSize);
            for (int u = 0; u < subPixelMatrixSize; u++)
            {
                double tempX = x;
                for (int v = 0; v < subPixelMatrixSize; v++)
                {
                    rect.at<cv::Vec3b>(u, v) = meanColWithSubPixels(img, tempX, tempY, nthWidth, nthHeight);
                    tempX += nthWidth;
                }
                tempY += nthHeight;
            }

            CharInfo best = findBestBlockCharacter(rect);

            for (int k = 0; k < 3; k++)
            {
                outMat[charIndex].foregroundRGB[k] = best.foregroundRGB[k];
                outMat[charIndex].backgroundRGB[k] = best.backgroundRGB[k];
            }
            outMat[charIndex].chara = best.chara;

            x += widthPixelsPerChar;
            charIndex++;
        }
        y += heightPixelsPerChar;
    }
}

}