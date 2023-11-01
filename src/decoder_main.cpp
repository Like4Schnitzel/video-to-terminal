#include "VTDIDecoder.hpp"

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

    VTDIDecoder player = VTDIDecoder(videoPath);
    player.getStaticInfo();

    std::cout << "VTDI Info:\n";
    std::cout << "Version: " << player.getVersion() << "\n";
    std::cout << "Frame count: " << player.getFrameCount() << "\n";
    std::cout << "FPS: " << player.getFPS() << "\n";
    std::cout << "Video dimensions: " << player.getVidWidth() << "x" << player.getVidHeight() << "\n";

    player.playVideo();

    return 0;
}