#include <map>
#include "videotranscoder.hpp"
#include "../libs/variousutils.hpp"

using namespace std;
using namespace vtt;

int main(int argc, char** argv)
{
    string videoPath;
    uint16_t tWidth;
    uint16_t tHeight;
    uint32_t memoryCap;
    uint threadCount;

    map<string, string> cliArgs;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-y") == 0 || strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "-Y") == 0 || strcmp(argv[i], "-N") == 0)
        {
            cliArgs.insert({"--skip", argv[i]});
        }
        else
        {
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
    if (fileEnding == ".vtdi")
    {
        throw invalid_argument("You entered the path to a vtdi file. This is the transcoder. Don't do that.");
    }

    string vtdiFilePath;
    if (cliArgs.count("--vtdi-path") > 0)
    {
        vtdiFilePath = cliArgs["--vtdi-path"];
    }
    else
    {
        vtdiFilePath = videoPath.substr(0, VariousUtils::rfind(videoPath, '.')) + ".vtdi";
    }

    if (VariousUtils::fileExists(vtdiFilePath))
    {
        if (cliArgs.count("--skip") == 0)
        {
            char option;
            cout << vtdiFilePath << " already exists. It will be overwritten if you continue. Continue anyways? [y/N] ";
            cin >> option;
            if (std::tolower(option) != 'y')
            {
                return 0;
            }
        }
        else if (cliArgs["--skip"] != "-y" && cliArgs["--skip"] != "-Y")
        {
            cout << vtdiFilePath << " already exists. Exiting.\n";
            return 0;
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

    if (cliArgs.count("--threads") > 0)
    {
        threadCount = VariousUtils::stringToInt(cliArgs["--threads"]);
    }
    else
    {
        threadCount = max(std::thread::hardware_concurrency(), 1);
    }

    VideoTranscoder trans(videoPath, vtdiFilePath, tWidth, tHeight);
    auto current = chrono::high_resolution_clock::now();
    trans.transcodeFile(1);
    cout << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - current) << "\n";

    return 0;
}