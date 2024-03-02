#pragma once

#include <array>
#include <vector>
#include <iostream>
#include "binaryutils.hpp"
#include "_kbhit.hpp"

#ifdef WIN32
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#endif

namespace vtt {

enum KeyPressValues {RIGHTARROW = 1792835, LEFTARROW = 1792836, V = 'V', ESC = 27};

struct KeyPress {
    bool keyDown;
    unsigned int keyValue;
};

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
        static KeyPress getKeyPress();
        static std::array<int, 2> getTerminalDimensions();
        void hideInput();
        void showInput();
};

}
