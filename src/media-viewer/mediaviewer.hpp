#pragma once

#include <filesystem>
#include <bits/stdc++.h>
#include "../transcoder/videotranscoder.hpp"
#include "../decoder/vtdidecoder.hpp"
#include "../img-viewer/imgviewer.hpp"

namespace vtt {

// Image, Encoded Video, Transcoded Video
enum FileType { IMG, VIDENC, VIDTRANS };

struct File {
    std::string path;
    FileType type;
};

class MediaViewer {
    private:
        std::vector<File> files;
        int filesIndex;
    public:
        MediaViewer(const std::filesystem::path path);
        const bool empty();
        /// @brief Get the currently selected file.
        /// @return A pointer to the file struct, or nullptr if the directory is empty.
        File* current();
        /// @brief Move to the next file in the list.
        /// @return A pointer to the next file in the list, or nullptr if the directory is empty.
        File* next();
        /// @brief Move to the previous file in the list.
        /// @return A pointer to the previous file in the list, or nullptr if the directory is empty.
        File* prev();
        void view();
};

}
