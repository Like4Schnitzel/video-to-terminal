#pragma once

#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include "SmartPtr.hpp"

class VariousUtils {
    public:
        static int rfind(std::string str, char c);
        static int stringToInt(std::string s);
        static bool fileExists(std::string fileName);

        template <typename T>
        static SmartPtr<T> subArray(SmartPtr<T> initArr, const int indexStart, const int indexEnd)
        {
            SmartPtr<T> sub = SmartPtr<T>(indexEnd-indexStart);
            int subIndex = 0;
            for (int i = indexStart; i < indexEnd; i++)
            {
                sub.set(subIndex, initArr.get(i));
                subIndex++;
            }
            return sub;
        }

        static SmartPtr<int> getTerminalDimensions();
        static char toLower(char c);
        
        template <typename T>
        static int find(const SmartPtr<T> inputArr, const T searchFor, const int indexStart)
        {
            for (int i = 0; i < inputArr.getSize(); i++)
            {
                if (inputArr.get(i) == searchFor)
                {
                    return i;
                }
            }
            return -1;
        }

        template <typename T>
        static void pushArrayToVector(const SmartPtr<T> inputArr, std::vector<T>* vec)
        {
            for (int i = 0; i < inputArr.getSize(); i++)
            {
                (*vec).push_back(inputArr.get(i));
            }
        }

        static std::string numToUnicodeBlockChar(int num);
};
