#include <ctype.h>
#include "mediaviewer.hpp"
#include "../libs/variousutils.hpp"
#include "../libs/termutils.hpp"
#include "../libs/binaryutils.hpp"

using namespace std;
using namespace vtt;

int main(int argc, char** argv)
{
    filesystem::path path;
    if (argc > 1)
        path = filesystem::relative(argv[1]);
    else
        path = filesystem::relative(filesystem::current_path());

    auto mv = MediaViewer(path);

    if (mv.empty())
    {
        cout << "The selected folder " << path << " does not contain viewable files.\n";
        return 0;
    }

    bool viewing = false;
    TermUtils tu;
    int fileIndex = 0;
    cout << "waiting for a keypress...\npress ESC to close the program.\n" << flush;
    cout << "Selected file: " << mv.current()->path << "\n" << flush;
    tu.hideInput();
    while (true)
    {
        KeyPress kp = TermUtils::getKeyPress();
        if (kp.keyDown)
        {
            if (!viewing && kp.keyValue == KeyPressValues::RIGHTARROW)
                cout << "Selected file: " << mv.next()->path << "\n";

            if (!viewing && kp.keyValue == KeyPressValues::LEFTARROW)
                cout << "Selected file: " << mv.prev()->path << "\n";

            // displaying a file
            if ((!viewing && std::toupper(kp.keyValue) == KeyPressValues::V) || mv.current()->type == FileType::IMG)
            {
                if (mv.current()->type != FileType::IMG)
                {
                    viewing = true;
                }

                auto sizeToUse = TermUtils::getTerminalDimensions();
                sizeToUse[1] -= 2; // leave space for the file name line
                mv.view(sizeToUse);
            }

            // ESC
            if (kp.keyValue == KeyPressValues::ESC)
            {
                if (viewing)
                    viewing = false;
                else
                    break;
            }
        }
    }
    tu.showInput();
    cout << "done\n";

    return 0;
}
