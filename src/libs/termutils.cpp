#include "termutils.hpp"

namespace vtt {

TermUtils::TermUtils()
{
    hidden = false;
}

// taken from https://stackoverflow.com/a/23370070
#ifdef WIN32

std::array<int, 2> TermUtils::getTerminalDimensions()
{
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns, rows;

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    std::array<int, 2> dimensions;
    dimensions[0] = columns;
    dimensions[1] rows;

    return dimensions;
}

void TermUtils::hideInput()
{
    if (!hidden)
    {
        this->hStdin = GetStdHandle(STD_INPUT_HANDLE); 
        this->mode = 0;
        GetConsoleMode(hStdin, &mode);
        SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
        hidden = true;
    }
}

void TermUtils::showInput()
{
    if (hidden)
    {
        SetConsoleMode(hStdin, mode);
        hidden = false;
    }
}
#else

std::array<int, 2> TermUtils::getTerminalDimensions()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    std::array<int, 2> dimensions;
    dimensions[0] = w.ws_col;
    dimensions[1] = w.ws_row;

    return dimensions;
}

void TermUtils::hideInput()
{
    if (!hidden)
    {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~ECHO;
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        hidden = true;
    }
}

void TermUtils::showInput()
{
    if (hidden)
    {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        hidden = false;
    }
}
#endif

}
