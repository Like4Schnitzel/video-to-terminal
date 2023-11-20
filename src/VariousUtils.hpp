#pragma once

#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <array>

class VariousUtils {
    public:
        static int rfind(std::string str, char c);
        static int stringToInt(std::string s);
        static bool fileExists(std::string fileName);

        template <typename T>
        static auto subArray(std::shared_ptr<T> initArr, const int indexStart, const int indexEnd)
        {
            std::vector<T> sub;
            sub.reserve(indexEnd-indexStart);
            int subIndex = 0;
            for (int i = indexStart; i < indexEnd; i++)
            {
                sub[subIndex] = initArr.get()[i]);
                subIndex++;
            }
            return sub;
        }

        static auto getTerminalDimensions();
        static char toLower(char c);
        
        template <typename T>
        static int find(const std::shared_ptr<T> inputArr, int inputArrLen, const T searchFor, const int indexStart)
        {
            for (int i = 0; i < inputArrLen; i++)
            {
                if (inputArr.get()[i] == searchFor)
                {
                    return i;
                }
            }
            return -1;
        }

        template <typename T>
        static void pushArrayToVector(std::shared_ptr<T> inputArr, int inputArrLen, std::vector<T>* vec)
        {
            for (int i = 0; i < inputArrLen; i++)
            {
                (*vec).push_back(inputArrLen);
            }
        }

        static std::string numToUnicodeBlockChar(int num);
};
