#pragma once

#ifdef WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#endif
#include <array>

namespace vtt {

class TermUtils {
    private:
        bool hidden;
#ifdef WIN32
        HANDLE hStdin;
        DWORD mode;
#else
        termios oldt;
        termios newt;
#endif
    public:
        TermUtils();
        static std::array<int, 2> getTerminalDimensions();
        void hideInput();
        void showInput();
};

}
