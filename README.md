# VideoToTerminal
Displays a video in the terminal.
The video first gets transcoded into a custom file format, VTDI (Video Terminal Display Information)
# VTDI Specification
# Version 1
The first 4 bytes are the file signature and when decoded as unsigned 8-bit integers should be 86 84 68 73\
The next 2 bytes are the file's version number as a uint.\
The next 4 bytes are the video's total frames as a uint.\
The next 4 bytes are the video's FPS as a float.\
The next 2 bytes give the transcoded video width within the terminal, followed by another 2 bytes for height.\
After that is the actual video display information.\
The video is displayed by using foreground and background ANSI RGB codes together with [Unicode Block Elements](https://en.wikipedia.org/wiki/Block_Elements).\
Frames are compressed using a custom compression algorithm.
Before every frame there is a 32 bit uint for the compressed size in bytes
, followed by another 32 bit uint for the uncompressed size.\
A frame is stored as a series of character information.\
Information for a character consists of 3 bytes for the foreground RGB values, 3 bytes for the background RGB values and 1 byte for the character.\
The character byte will take a value from 0 to 31, and will signify a Unicode character from U+2580 to U+259F.\
## VTDI Compression Specification
Every frame is compressed to simply contain the bytes for a CharInfo, followed by 2 bits:
- 00 if a rectangle is following; The rectangle will consist of 2 bytes each for x and y position of the upper left corner and then again for the lower right corner.
- 01 if a single position is following; The position is given by 2 bytes for x and y respectively.
- 10 for the end of the CharInfo; New CharInfoBytes follow this.
- 11 for the end of the frame. \
  After 11 there will be up to 7 0-bits for padding. \
  After the padding there will be either:
  - 0, marking that the next frame contains new information.
  - 1, marking that the next frame is the same as the current one. 
Areas that haven't changed from the last frame won't be listed.
# Version 2
Misaligned bits have been replaced with padded bytes for speed reasons.\
What this means is that the signifier bits after a CharInfo now look like this:
- 00 -> 00000000
- 01 -> 00000001
- 10 -> 00000010
- 11 -> 00000011

The 0- or 1-bit after 00000011 are now also 00000000 and 00000001 respectively.
