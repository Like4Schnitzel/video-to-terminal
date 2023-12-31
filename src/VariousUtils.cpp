#include "VariousUtils.hpp"

int VariousUtils::rfind(std::string str, char c)
{
    int i;
    for (i = str.length()-1; i >= 0; i--)
    {
        if (str[i] == c)
        {
            break;
        }
    }
    return i;
}

int VariousUtils::stringToInt(std::string s)
{
    int n = 0;
    int mult = 1;
    for (int i = s.length() - 1; i >= 0; i--)
    {
        if (s[i] < '0' || s[i] > '9')
        {
            throw std::invalid_argument("Input must be a number.");
        }
        n += (s[i] - '0') * mult;
        mult *= 10;
    }

    return n;
}

bool VariousUtils::fileExists(std::string fileName)
{
    struct stat buffer;
    return (stat (fileName.c_str(), &buffer) == 0);
}


// taken from https://stackoverflow.com/a/23370070
#ifdef WIN32
#include <windows.h>

std::array<int, 2> VariousUtils::getTerminalDimensions()
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
#else
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

std::array<int, 2> VariousUtils::getTerminalDimensions()
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    std::array<int, 2> dimensions;
    dimensions[0] = w.ws_col;
    dimensions[1] = w.ws_row;

    return dimensions;
}
#endif

char VariousUtils::toLower(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c - 32;
    }
    else
    {
        return c;
    }
}

std::string VariousUtils::numToUnicodeBlockChar(int num)
{
    char buf[3];
    buf[0] = 0xe2;
    buf[1] = 0x96;
    buf[2] = 0x80 + num;
    return std::string(reinterpret_cast<char*>(buf), 3);
}
