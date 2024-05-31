#include "imgviewer.hpp"

namespace vtt {

ImgViewer::ImgViewer(const std::string path)
{
    file = cv::imread(path);
}

void ImgViewer::transcode(const int width, const int height)
{
    this->width = width;
    this->height = height;

    transcodedFile.resize(width*height);
    imgToCIMat(this->file, width, height, this->transcodedFile);
}

void ImgViewer::print()
{
    if (transcodedFile.size() == 0) throw std::logic_error("File has not been transcoded yet.");

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

int ImgViewer::getPixelWidth()
{
    return file.size().width;
}

int ImgViewer::getPixelHeight()
{
    return file.size().height;
}

int ImgViewer::getTerminalWidth()
{
    return width;
}

int ImgViewer::getTerminalHeight()
{
    return height;
}

}
