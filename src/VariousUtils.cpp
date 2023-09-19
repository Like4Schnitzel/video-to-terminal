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
