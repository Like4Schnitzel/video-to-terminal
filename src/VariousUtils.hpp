#pragma once

#include <iostream>
#include <sys/stat.h>
#include <unistd.h>

class VariousUtils {
    public:
        static int rfind(std::string str, char c);
        static int stringToInt(std::string s);
        static bool fileExists(std::string fileName);
};
