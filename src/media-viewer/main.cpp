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
            if (!viewing && kp.keyValue == 1792835)
                cout << "Selected file: " << mv.next()->path << "\n";

            // left arrow
            if (!viewing && kp.keyValue == 1792836)
                cout << "Selected file: " << mv.prev()->path << "\n";

            // V key
            if (!viewing && std::toupper(kp.keyValue) == 'V')
            {
                viewing = true;
                mv.view();
            }

            // ESC
            if (kp.keyValue == 27)
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
