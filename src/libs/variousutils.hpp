#pragma once

#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <array>
#include <memory>

namespace vtt {

class VariousUtils {
    public:
        static int rfind(std::string str, char c);
        static int stringToInt(std::string s);
        static bool fileExists(std::string fileName);

        template <typename T>
        static auto subArray(const T* initArr, const int indexStart, const int indexEnd)
        {
            std::vector<T> sub;
            sub.reserve(indexEnd-indexStart);
            for (int i = indexStart; i < indexEnd; i++)
            {
                sub.push_back(initArr[i]);
            }
            return sub;
        }

        static std::array<int, 2> getTerminalDimensions();
        static char toLower(char c);

        template <typename T>
        static void pushArrayToVector(const T* inputArr, int inputArrLen, std::vector<T>& vec)
        {
            for (int i = 0; i < inputArrLen; i++)
            {
                vec.push_back(inputArr[i]);
            }
        }

        static std::string numToUnicodeBlockChar(int num);
};

}
