#include "charInfo.hpp"

CharInfo::CharInfo()
{
    foregroundRGB = (uint8_t*)malloc(3);
    backgroundRGB = (uint8_t*)malloc(3);
}

CharInfo::~CharInfo()
{
    free(foregroundRGB);
    free(backgroundRGB);
}
