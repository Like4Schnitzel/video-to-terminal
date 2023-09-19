#pragma once

#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

class VariousUtils {
    public:
        static int rfind(std::string str, char c);
        static int stringToInt(std::string s);
        static bool fileExists(std::string fileName);

        template <typename T>
        static T* subArray(const T* initArr, const int indexStart, const int indexEnd)
        {
            T* sub = (T*)malloc((indexEnd-indexStart + 1) * sizeof(T));
            int subIndex = 0;
            for (int i = indexStart; i <= indexEnd; i++)
            {
                sub[subIndex] = initArr[indexStart];
                subIndex++;
            }
            return sub;
        }
};
