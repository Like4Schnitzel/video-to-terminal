#include "_kbhit.hpp"

#ifndef WIN32

// taken from https://www.flipcode.com/archives/_kbhit_for_Linux.shtml
// very slightly modified in the header file (#include <strops.h> to #include <sys/ioctl.h>)

/**
 Linux (POSIX) implementation of _kbhit().
 Morgan McGuire, morgan@cs.brown.edu
*/

int _kbhit()  
{
    static const int STDIN = 0;
    static bool initialized = false;

    if (! initialized)
    {
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
