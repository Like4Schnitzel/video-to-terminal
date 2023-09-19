#include "VideoTranscoder.hpp"
#include "VTDIDecoder.hpp"
#include "VariousUtils.hpp"

using namespace std;

int main(int argc, char** argv)
{
    string videoPath;
    uint16_t tWidth;
    uint16_t tHeight;
    if (argc > 1)
    {
        videoPath = argv[1];
    }
    else 
    {
        cout << "Please enter the path to the video you want to play: ";
        cin >> videoPath;
    }

    const std::string vtdiFilePath = videoPath.substr(0, VariousUtils::rfind(videoPath, '.')) + ".vtdi";
    if (!VariousUtils::fileExists(vtdiFilePath))
    {
        if (argc > 2)
        {
            tWidth = VariousUtils::stringToInt(argv[2]);
        }
        else
        {
            cout << "Please enter a 16 bit unsigned int for the terminal width: ";
            cin >> tWidth;
        }
        if (argc > 3)
        {
            tHeight = VariousUtils::stringToInt(argv[3]);
        }
        else
        {
            cout << "Please enter a 16 bit unsigned int for the terminal height: ";
            cin >> tHeight;
        }

        VideoTranscoder trans = VideoTranscoder(videoPath, tWidth, tHeight);
        trans.transcodeFile();
    }

    VTDIDecoder player = VTDIDecoder(vtdiFilePath);
    player.getStaticInfo();

    return 0;
}
