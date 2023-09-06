#include "VideoTranscoder.hpp"

using namespace std;

int main()
{
    std::string videoPath;
    cout << "Please enter the path to the video you want to play: ";
    cin >> videoPath;

    VideoTranscoder trans = VideoTranscoder(videoPath);
    cv::namedWindow("Video");
    cv::imshow("Video", trans.getFrame());

    return 0;
}
