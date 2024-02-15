#include <iostream>
#include <termio.h>
#include "../libs/variousutils.hpp"
#include "../libs/termutils.hpp"
#include "../libs/binaryutils.hpp"
#include "../libs/_kbhit.h"

using namespace std;
using namespace vtt;

int main(int argc, char** argv)
{
    filesystem::path path;
    if (argc > 1)
    {
        path = filesystem::absolute(argv[1]);
        if (!filesystem::is_directory(path))
            path = path.parent_path();
    }
    else
        path = filesystem::current_path();

    auto files = VariousUtils::getFilesInDir(path);

    TermUtils tu;
    int fileIndex = 0;
    cout << "Selected file: " << files[fileIndex].d_name << "\n";
    unsigned char keyValue;
    cout << "waiting for a keypress...\npress ESC to close the program.\n" << flush;
    tu.hideInput();
    while (true)
    {
        if (_kbhit())
        {
            std::vector<char> vals;
            while (_kbhit())
            {
                cin >> keyValue;
                vals.push_back(keyValue);
            }
            int combinedKeyCode = BinaryUtils::byteArrayToUint((Byte*)vals.data(), vals.size());

            // right arrow
            if (combinedKeyCode == 1792835)
            {
                fileIndex = (fileIndex + 1) % files.size();
                cout << "Selected file: " << files[fileIndex].d_name << "\n";
            }

            // left arrow
            if (combinedKeyCode == 1792836)
            {
                fileIndex--;
                if (fileIndex < 0) fileIndex += files.size();
                cout << "Selected file: " << files[fileIndex].d_name << "\n";
            }

            // ESC
            if (combinedKeyCode == 27)
                break;
        }
    }
    tu.showInput();
    cout << "done\n";

    return 0;
}
