#include "VideoTranscoder.hpp"

using namespace std;

int main(int argc, char** argv)
{
    string videoPath;
    if (argc > 1)
    {
        videoPath = argv[1];
    }
    else 
    {
        cout << "Please enter the path to the video you want to play: ";
        cin >> videoPath;
    }

    VideoTranscoder trans = VideoTranscoder(videoPath);
    trans.transCodeFile();

    return 0;
}
