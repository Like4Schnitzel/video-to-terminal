#include "VideoTranscoder.hpp"

using namespace std;

int stringToInt(std::string s)
{
    int n = 0;
    int mult = 1;
    for (int i = s.length() - 1; i >= 0; i--)
    {
        if (s[i] < '0' || s[i] > '9')
        {
            throw invalid_argument("Input must be a number.");
        }
        n += (s[i] - '0') * mult;
        mult *= 10;
    }

    return n;
}

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
    if (argc > 2)
    {
        tWidth = stringToInt(argv[2]);
    }
    else
    {
        cout << "Please enter a 16 bit unsigned int for the terminal width.";
        cin >> tWidth;
    }
    if (argc > 3)
    {
        tHeight = stringToInt(argv[3]);
    }
    else
    {
        cout << "Please enter a 16 bit unsigned int for the terminal height.";
        cin >> tHeight;
    }

    VideoTranscoder trans = VideoTranscoder(videoPath, tWidth, tHeight);
    trans.transcodeFile();
    trans.getFrame();
    trans.transcodeFrame();

    return 0;
}
