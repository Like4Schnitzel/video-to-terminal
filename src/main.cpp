#include "VideoTranscoder.hpp"
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui.hpp>

using namespace std;

int main()
{
    string videoPath;
    cout << "Please enter the path to the video you want to play: ";
    cin >> videoPath;

    VideoTranscoder trans = VideoTranscoder(videoPath);
    cv::Mat currentFrame = trans.getFrame();
    while (!currentFrame.empty()) 
    {
        cv::imshow("Frame", currentFrame);
        if (cv::waitKey(33) != -1)
        {
            break;
        }
        currentFrame = trans.getFrame();
    }

    return 0;
}
