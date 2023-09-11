#pragma once

#include <cstdint>
#include <cstdlib>

class CharInfo {
    public:
        uint8_t* foregroundRGB;
        uint8_t* backgroundRGB;
        uint8_t chara;
        
        CharInfo();
        ~CharInfo();
};
