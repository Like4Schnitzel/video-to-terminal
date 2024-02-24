#pragma once

#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>
#include "../libs/charinfo.hpp"
#include "../libs/variousutils.hpp"
#include "../libs/binaryutils.hpp"
#include "../libs/termutils.hpp"

namespace vtt {

class VTDIDecoder {
    private:
        int staticByteSize;
        std::string vtdiPath;
        std::ifstream vtdiFile;
        std::array<char, 7> buffer;
        std::unique_ptr<CharInfo[]> currentFrame;
        uint32_t frameCount;
        uint16_t vidWidth;
        uint16_t vidHeight;
        int terminalWidth;
        int terminalHeight;
        float FPS;
        uint16_t version;
    public:
        VTDIDecoder(std::string path, bool debugInfo = true);
        void readStaticInfo();
        void playVideo();
        void readAndDisplayNextFrame(bool display=true, bool save=true);
        void displayCurrentFrame();

        int getVersion();
        uint32_t getFrameCount();
        float getFPS();
        int getVidWidth();
        int getVidHeight();
};

}
