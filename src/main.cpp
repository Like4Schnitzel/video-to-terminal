#include "VideoTranscoder.hpp"

using namespace std;

int main()
{
    string videoPath;
    cout << "Please enter the path to the video you want to play: ";
    cin >> videoPath;

    VideoTranscoder trans = VideoTranscoder(videoPath);
    trans.transCodeFile();
    cout << "FPS is " << trans.getFPS() << "\n";

    return 0;
}
