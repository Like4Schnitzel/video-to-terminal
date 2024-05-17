#include "mediaviewer.hpp"

namespace vtt {

bool validVTDI(std::string path)
{
    auto decoder = VTDIDecoder(path, false);
    try
    {
        decoder.readStaticInfo(false);
    }
    catch (std::runtime_error)
    {
        return false;
    }

    return true;
}

File* MediaViewer::readFile(std::string filePath)
{
    if (!cv::imread(filePath).empty())
    {
        File* fileStruct = new File;
        (*fileStruct).path = filePath;
        (*fileStruct).type = FileType::IMG;

        return fileStruct;
    }
    else if (cv::VideoCapture(filePath).isOpened())
    {
        File* fileStruct = new File;

        std::string vtdiPath = ((std::string)filePath).substr(0, VariousUtils::rfind(filePath, '.')) + ".vtdi";
        if (std::filesystem::exists(vtdiPath) && validVTDI(vtdiPath))
        {
            (*fileStruct).path = vtdiPath;
            (*fileStruct).type = FileType::VIDTRANS;
        }
        else
        {
            (*fileStruct).path = filePath;
            (*fileStruct).type = FileType::VIDENC;
        }

        return fileStruct;
    }
    else if (validVTDI(filePath))
    {
        // VTDI file might have already been added in a previous mp4 check. Make sure it's not a dupe.
        if (!std::none_of(fileCache.begin(), fileCache.end(), [&filePath](File d){return std::strcmp(d.path.c_str(), filePath.c_str()) == 0;}))
        {
            return nullptr;
        }

        File* fileStruct = new File;
        (*fileStruct).path = filePath;
        (*fileStruct).type = FileType::VIDTRANS;

        return fileStruct;
    }

    // this is practically an else
    return nullptr;
}

MediaViewer::MediaViewer(const std::filesystem::path path, int maxCacheSize)
{
    this->maxCacheSize = maxCacheSize;

    bool checkForFile;
    if (std::filesystem::is_directory(path))
    {
        checkForFile = false;
        dirPath = path;
    }
    else
    {
        checkForFile = true;
        dirPath = path.parent_path();
    }

    unfilteredFiles = VariousUtils::getFilesInDir(dirPath);
    sort(unfilteredFiles.begin(), unfilteredFiles.end(), [](dirent d1, dirent d2){return (std::string)d1.d_name < (std::string)d2.d_name;});

    int startIndex = 0;
    if (checkForFile)
    {
        for (int i = 0; i < unfilteredFiles.size(); i++)
        {
            if (dirPath / unfilteredFiles[i].d_name == path)
            {
                startIndex = i;
                break;
            }
        }
    }

    // used later if the file is found
    int i;
    for (i = startIndex; i < unfilteredFiles.size() && fileCache.size() < maxCacheSize / 2; i++)
    {
        auto filePath = dirPath / unfilteredFiles[i].d_name;
        auto file = readFile(filePath);

        if (file != nullptr)
        {
            (*file).unfilteredFilesIndex = i;
            fileCache.push_back(*file);
        }
    }

    if (i < unfilteredFiles.size())
    {
        for (int j = (startIndex-1+unfilteredFiles.size()) % unfilteredFiles.size();
             j != i && fileCache.size() < maxCacheSize;
             j = (j - 1 + unfilteredFiles.size()) % unfilteredFiles.size())
        {
            auto filePath = dirPath / unfilteredFiles[j].d_name;
            auto file = readFile(filePath);

            if (file != nullptr)
            {
                (*file).unfilteredFilesIndex = j;
                fileCache.insert(fileCache.begin(), *file);
            }
        }
    }

    filesIndex = fileCache.size() / 2;
}

const bool MediaViewer::empty()
{
    return fileCache.empty();
}

File* MediaViewer::current()
{
    if (fileCache.empty()) return nullptr;

    return &fileCache[filesIndex];
}

File* MediaViewer::next()
{
    if (fileCache.empty()) return nullptr;
    
    // remove file at first index and then add one to the back, we won't need to change the file index
    // for this we shift everything to the left by one
    for (int i = 1; i < fileCache.size(); i++)
        fileCache[i-1] = fileCache[i];

    // find next file after the current last one
    File* newLast = nullptr;
    for (int i = fileCache.back().unfilteredFilesIndex+1;
         newLast == nullptr && i != fileCache.back().unfilteredFilesIndex;
         i = (i + 1) % unfilteredFiles.size())
    {
        newLast = readFile(dirPath / unfilteredFiles[i].d_name);

        if (newLast != nullptr)
        {
            (*newLast).unfilteredFilesIndex = i;
        }
    }
    fileCache.back() = *newLast;

    if (newLast == nullptr)
        throw std::runtime_error("Couldn't find new next file.");

    return &fileCache[filesIndex];
}

File* MediaViewer::prev()
{
    if (fileCache.empty()) return nullptr;

    // remove file at last index and then add one at the front, file index doesn't need to be changed
    for (int i = fileCache.size()-1; i >= 1; i--)
        fileCache[i] = fileCache[i-1];

    // find new first file before the current first one
    File* newFirst = nullptr;
    for (int i = fileCache.front().unfilteredFilesIndex-1;
         newFirst == nullptr && i != fileCache.front().unfilteredFilesIndex;
         i = (i - 1 + unfilteredFiles.size()) % unfilteredFiles.size())
    {
        newFirst = readFile(dirPath / unfilteredFiles[i].d_name);

        if (newFirst != nullptr)
        {
            (*newFirst).unfilteredFilesIndex = i;
        }
    }
    if (newFirst == nullptr)
        throw std::runtime_error("Couldn't find new previous file.");

    fileCache.front() = *newFirst;

    return &fileCache[filesIndex];
}

ViewExitCode MediaViewer::view(std::array<int, 2> maxDims, bool ignoreWarning)
{
    auto currentFile = current();
    switch (currentFile->type)
    {
        case FileType::IMG:
        {
            auto viewer = ImgViewer(currentFile->path);
            auto pDims = std::array<int, 2>();
            pDims[0] = viewer.getPixelWidth();
            pDims[1] = viewer.getPixelHeight();

            double aspectRatio = (double) pDims[0] / pDims[1];
            std::array<double, 2> tDims;

            // try maxing out height
            tDims[1] = maxDims[1];
            tDims[0] = 2 * aspectRatio * tDims[1];

            // if that doesn't work, max out width
            if (tDims[0] > maxDims[0])
            {
                tDims[0] = maxDims[0];
                tDims[1] = 0.5 * tDims[0] / aspectRatio;
            }

            viewer.transcode(tDims[0], tDims[1]);
            viewer.print();
            break;
        }

        case FileType::VIDTRANS:
        {
            auto viewer = VTDIDecoder(currentFile->path, false);
            auto tDims = TermUtils::getTerminalDimensions();
            viewer.readStaticInfo(false);

            if (!ignoreWarning && (viewer.getVidWidth() > tDims[0] || viewer.getVidHeight() > tDims[1]))
            {
                return ViewExitCode::TERMINALTOOSMALL;
            }

            else
            {
                viewer.playVideo();
            }

            break;
        }

        default:
        {
            return ViewExitCode::FILETYPEUNKNOWN;
        }
    }

    return ViewExitCode::ALLGOOD;
}

int MediaViewer::getIndex()
{
    return filesIndex;
}

}
