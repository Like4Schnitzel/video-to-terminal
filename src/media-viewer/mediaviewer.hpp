#pragma once

#include <filesystem>
#include <bits/stdc++.h>
#include "../transcoder/videotranscoder.hpp"
#include "../decoder/vtdidecoder.hpp"
#include "../img-viewer/imgviewer.hpp"

namespace vtt {

// Image, Encoded Video, Transcoded Video
enum FileType { IMG, VIDENC, VIDTRANS };
enum class ViewExitCode { ALLGOOD, FILETYPEUNKNOWN, TERMINALTOOSMALL };
const std::map<ViewExitCode, std::string> ExitCodes
{ 
    {ViewExitCode::ALLGOOD, "All good."},
    {ViewExitCode::FILETYPEUNKNOWN, "File type unknown."},
    {ViewExitCode::TERMINALTOOSMALL, "The terminal's size is too small to display the video."}
};

struct File {
    std::filesystem::path path;
    FileType type;
    int unfilteredFilesIndex = -1;
};

class MediaViewer {
    private:
        std::vector<File> fileCache;
        int maxCacheSize;
        std::vector<dirent> unfilteredFiles;
        int filesIndex;
        File* readFile(std::filesystem::path filePath);
        std::filesystem::path dirPath;
    public:
        MediaViewer(const std::filesystem::path path, int maxCacheSize);
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
        /// @brief Displays the currently selected file's contents in the terminal.
        /// @param maxDims The maximum allowed size to use for the displaying. The image will be scaled to this as far as possible while staying close to the original aspect ratio. Defaults to the size of the terminal.
        /// @param ignoreWarning Whether or not to ignore warnings. If this is false, warnings will return with their corresponding exit code.
        ViewExitCode view(std::array<int, 2> maxDims = TermUtils::getTerminalDimensions(), bool ignoreWarning = false);
        int getIndex();
};

}
