#include "VideoTranscoder.hpp"
#include "VTDIDecoder.hpp"
#include "VariousUtils.hpp"
#include <map>

using namespace std;

int memCapInputFormat(string s)
{
    std::map<char, int> prefixes = {
        {'k', 1000},
        {'m', 1000000},
        {'g', 1000000000}
    };

    char lastChar = VariousUtils::toLower(s.back());
    if (lastChar >= '0' && lastChar <= '9')
    {
        return VariousUtils::stringToInt(s);
    }
    else if (lastChar == 'k' || lastChar == 'm' || lastChar == 'g')
    {
        return VariousUtils::stringToInt(s.substr(0, s.length()-1)) * prefixes[lastChar];
    }
    else
    {
        throw std::runtime_error("Invalid input format.");
    }
}

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
        if (argc > 4)
        {
            memoryCap = memCapInputFormat(argv[4]);
        }
        else
        {
            std::string memCapString;
            cout << "Please enter memory capacity in bytes: ";
            cin >> memCapString;

            memoryCap = memCapInputFormat(memCapString);
        }

        VideoTranscoder trans = VideoTranscoder(videoPath, tWidth, tHeight, memoryCap);
        trans.transcodeFile();
    }

    VTDIDecoder player = VTDIDecoder(vtdiFilePath);
    player.getStaticInfo();

    std::cout << "VTDI Info:\n";
    std::cout << "Version: " << player.getVersion() << "\n";
    std::cout << "Frame count: " << player.getFrameCount() << "\n";
    std::cout << "FPS: " << player.getFPS() << "\n";
    std::cout << "Video dimensions: " << player.getVidWidth() << "x" << player.getVidHeight() << "\n";

    player.playVideo();

    return 0;
}
