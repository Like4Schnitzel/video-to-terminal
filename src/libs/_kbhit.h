#pragma once

#ifdef WIN32

// _kbhit() is originally from this library but it is windows only
#include <conio.h>

#else
// taken from https://www.flipcode.com/archives/_kbhit_for_Linux.shtml
// very slightly modified (#include <strops.h> to #include <sys/ioctl.h>)

/**
 Linux (POSIX) implementation of _kbhit().
 Morgan McGuire, morgan@cs.brown.edu
*/

#include <stdio.h>
#include <sys/select.h>
#include <termios.h>
#include <sys/ioctl.h>

int _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (! initialized) {
        // Use termios to turn off line buffering
        termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}
#endif
