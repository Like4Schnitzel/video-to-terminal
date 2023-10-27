#include "VideoTranscoder.hpp"
#include "VTDIDecoder.hpp"
#include "VariousUtils.hpp"

using namespace std;

int main(int argc, char** argv)
{
    string videoPath;
    uint16_t tWidth;
    uint16_t tHeight;
    uint32_t memoryCap;
    if (argc > 1)
    {
        videoPath = argv[1];
    }
    else 
    {
        cout << "Please enter the path to the video you want to play: ";
        cin >> videoPath;
    }

    const int fileEndingStart = VariousUtils::rfind(videoPath, '.');
    const string fileEnding = videoPath.substr(fileEndingStart);
    if (fileEnding != ".vtdi")
    {
        const string vtdiFilePath = videoPath.substr(0, VariousUtils::rfind(videoPath, '.')) + ".vtdi";
        if (VariousUtils::fileExists(vtdiFilePath))
        {
            char option;
            cout << vtdiFilePath << " already exists. It will be overwritten if you continue. Continue anyways? [y/N] ";
            cin >> option;
            if (VariousUtils::toLower(option) != 'y')
            {
                return 0;
            }
        }

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

    else
    {
        VTDIDecoder player = VTDIDecoder(videoPath);
        player.getStaticInfo();

        std::cout << "VTDI Info:\n";
        std::cout << "Version: " << player.getVersion() << "\n";
        std::cout << "Frame count: " << player.getFrameCount() << "\n";
        std::cout << "FPS: " << player.getFPS() << "\n";
        std::cout << "Video dimensions: " << player.getVidWidth() << "x" << player.getVidHeight() << "\n";

        player.playVideo();
    }

    return 0;
}
