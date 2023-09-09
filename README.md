# VideoToTerminal
Displays a video in the terminal.
The video first gets transcoded into a custom file format, VTDI (Video Terminal Display Information)
### VTDI Specification
The first 4 bytes are the file signature and when decoded as unsigned 8-bit integers should be 86 84 68 73\
The next 4 bytes are the video's FPS as a float.\
The next 2 bytes are an unsigned integer giving the video's width in pixels, followed by another 2 bytes for height.\
The next 4 bytes are a uint giving the value of the rest of the file's uncompressed size in bits.\
After that is the actual video display information, compressed by deflating.
