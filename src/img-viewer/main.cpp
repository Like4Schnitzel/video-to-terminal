#include <iostream>
#include "imgviewer.hpp"

using namespace std;
using namespace vtt;

int main(int argc, char** argv)
{
    std::string filePath;
    int width;
    int height;

    if (argc > 1)
    {
        filePath = argv[1];
    }
    else
    {
        cout << "Please enter a path to an image: ";
        cin >> filePath;
    }

    if (argc <= 3) cout << "Original size: " << cv::imread(filePath).size() << "\n";

    if (argc > 2)
    {
        width = VariousUtils::stringToInt(argv[2]);
    }
    else
    {
        cout << "Please enter width: ";
        cin >> width;
    }

    if (argc > 3)
    {
        height = VariousUtils::stringToInt(argv[3]);
    }
    else
    {
        cout << "Please enter height: ";
        cin >> height;
    }

    ImgViewer viewer(filePath);
    viewer.transcode(width, height);
    viewer.print();

    return 0;
}
