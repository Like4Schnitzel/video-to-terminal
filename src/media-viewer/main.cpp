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
        path = filesystem::absolute(argv[1]);
    else
        path = filesystem::current_path();

    auto mv = MediaViewer(path);

    if (mv.empty())
    {
        cout << "The selected folder " << path << " does not contain viewable files.\n";
        return 0;
    }

    TermUtils tu;
    int fileIndex = 0;
    cout << "Selected file: " << mv.current()->path << "\n" << flush;
    unsigned char keyValue;
    cout << "waiting for a keypress...\npress ESC to close the program.\n" << flush;
    tu.hideInput();
    while (true)
    {
        KeyPress kp = TermUtils::getKeyPress();
        if (kp.keyDown)
        {
            // right arrow
            if (kp.keyValue == 1792835)
                cout << "Selected file: " << mv.next()->path << "\n";

            // left arrow
            if (kp.keyValue == 1792836)
                cout << "Selected file: " << mv.current()->path << "\n";

            // ESC
            if (kp.keyValue == 27)
                break;
        }
    }
    tu.showInput();
    cout << "done\n";

    return 0;
}
