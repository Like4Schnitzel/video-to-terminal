#include "vtdidecoder.hpp"
#include <string.h>

using namespace std;
using namespace vtt;

int main(int argc, char** argv)
{
    bool loop = false;
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
    if (argc > 2 && strcmp(argv[2], "--loop") == 0)
    {
        loop = true;
    }

    VTDIDecoder player = VTDIDecoder(videoPath);
    player.readStaticInfo();

    std::cout << "VTDI Info:\n";
    std::cout << "Version: " << player.getVersion() << "\n";
    std::cout << "Frame count: " << player.getFrameCount() << "\n";
    std::cout << "FPS: " << player.getFPS() << "\n";
    std::cout << "Video dimensions: " << player.getVidWidth() << "x" << player.getVidHeight() << "\n";

    do
    {
        player.playVideo();
    } while (loop);

    return 0;
}