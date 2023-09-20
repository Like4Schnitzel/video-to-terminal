# VideoToTerminal
Displays a video in the terminal.
The video first gets transcoded into a custom file format, VTDI (Video Terminal Display Information)
### VTDI Specification
The first 4 bytes are the file signature and when decoded as unsigned 8-bit integers should be 86 84 68 73\
The next 2 bytes are the file's version number as a uint.\
The next 4 bytes are the video's total frames as a uint.\
The next 4 bytes are the video's FPS as a float.\
The next 2 bytes give the transcoded video width within the terminal, followed by another 2 bytes for height.\
The next 8 bytes are a ulong giving the value of the frame information part's uncompressed size in bytes.\
The next 8 bytes are a ulong giving the value of the frame information part's compressed size in bytes.\
After that is the actual video display information, compressed by deflating.\
The video is displayed by using foreground and background ANSI RGB codes together with [Unicode Block Elements](https://en.wikipedia.org/wiki/Block_Elements).\
A frame is stored as a series of character information.\
Information for a character consists of 3 bytes for the foreground RGB values, 3 bytes for the background RGB values and 1 byte for the character.\
The character byte will take a value from 0 to 31, and will signify a Unicode character from U+2580 to U+259F.\
A frame is ended with the character information byte sequence 0 0 0 0 0 0 32.
