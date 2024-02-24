#pragma once

#ifdef WIN32

// _kbhit() is originally from this library but it is windows only
#include <conio.h>

#else

#include <stdio.h>
#include <sys/select.h>
#include <termios.h>
#include <sys/ioctl.h>

int _kbhit();

#endif
