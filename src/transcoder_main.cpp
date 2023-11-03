#include <map>
#include "VideoTranscoder.hpp"
#include "VariousUtils.hpp"

using namespace std;

int main(int argc, char** argv)
{
    string videoPath;
    uint16_t tWidth;
    uint16_t tHeight;
    uint32_t memoryCap;

    map<string, string> cliArgs;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-y") == 0 || strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "-Y") == 0 || strcmp(argv[i], "-N") == 0)
        {
            cout << "got into --skip with " << argv[i] << "\n";
            cliArgs.insert({"--skip", argv[i]});
        }
        else
        {
            std::cout << argv[i] << argv[i+1] << "\n";
            cliArgs.insert({argv[i], argv[i+1]});
            i++;
        }
    }

    if (cliArgs.count("--path") > 0)
    {
        videoPath = cliArgs["--path"];
    }
    else 
    {
        cout << "Please enter the path to the video you want to transcode: ";
        cin >> videoPath;
    }

    const int fileEndingStart = VariousUtils::rfind(videoPath, '.');
    const string fileEnding = videoPath.substr(fileEndingStart);
    if (fileEnding != ".vtdi")
    {
        const string vtdiFilePath = videoPath.substr(0, VariousUtils::rfind(videoPath, '.')) + ".vtdi";
        if (VariousUtils::fileExists(vtdiFilePath))
        {
            if ((cliArgs.count("--skip") > 0 && cliArgs["--skip"] != "-y"))
            {
                char option;
                cout << vtdiFilePath << " already exists. It will be overwritten if you continue. Continue anyways? [y/N] ";
                cin >> option;
                if (VariousUtils::toLower(option) != 'y')
                {
                    return 0;
                }
            }
        }

        if (cliArgs.count("--width") > 0)
        {
            tWidth = VariousUtils::stringToInt(cliArgs["--width"]);
        }
        else
        {
            cout << "Please enter a 16 bit unsigned int for the terminal width: ";
            cin >> tWidth;
        }
        if (cliArgs.count("--height") > 0)
        {
            tHeight = VariousUtils::stringToInt(cliArgs["--height"]);
        }
        else
        {
            cout << "Please enter a 16 bit unsigned int for the terminal height: ";
            cin >> tHeight;
        }

        VideoTranscoder trans = VideoTranscoder(videoPath, tWidth, tHeight);
        trans.transcodeFile();
    }

    return 0;
}