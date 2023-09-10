#pragma once

#include <vector>
#include <cstdlib>
#include <cstdint>

class BinaryUtils {
    public:
        static void push_array(std::vector<bool>, bool*, int);
        static bool* uint8ToBitArray(uint8_t);
};