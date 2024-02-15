#pragma once

#include "../transcoder/videotranscoder.hpp"
#include "../decoder/vtdidecoder.hpp"
#include "../img-viewer/imgviewer.hpp"

namespace vtt {

enum FileType { image, video };

struct File {
    std::string path;
    FileType type;
    bool readyToView;
};

class MediaViewer {
    private:
        std::vector<File> files;
};

}
