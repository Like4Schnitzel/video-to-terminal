#include "variousutils.hpp"

namespace vtt {

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

std::string VariousUtils::numToUnicodeBlockChar(int num)
{
    char buf[3];
    buf[0] = 0xe2;
    buf[1] = 0x96;
    buf[2] = 0x80 + num;
    return std::string(reinterpret_cast<char*>(buf), 3);
}

std::vector<dirent> VariousUtils::getFilesInDir(std::filesystem::path path)
{
    if (!std::filesystem::is_directory(path))
    {
        std::stringstream errorMsg;
        errorMsg << path.c_str() << " is not a directory.";
        throw std::runtime_error(errorMsg.str());
    }

    std::vector<dirent> files;
    DIR* dp = nullptr;
    dirent* entry = nullptr;
    dp = opendir(path.c_str());

    while ((entry = readdir(dp)))
    {
        // d_type 8 is file. 4 is directory.
        if (entry->d_type == 8)
        {
            files.push_back(*entry);
        }
    }

    return files;
}

}
