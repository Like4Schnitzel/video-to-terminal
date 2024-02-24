#include "mediaviewer.hpp"

namespace vtt {

bool validVTDI(std::string path)
{
    auto decoder = VTDIDecoder(path, false);
    try
    {
        decoder.readStaticInfo();
    }
    catch (std::runtime_error)
    {
        return false;
    }

    return true;
}

MediaViewer::MediaViewer(const std::filesystem::path path)
{
    filesIndex = 0;

    bool checkForFile;
    std::filesystem::path dirPath;
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

    int i = 0;
    for (const auto file : VariousUtils::getFilesInDir(dirPath))
    {
        auto dirPathCopy = dirPath;
        std::string pathFileName = dirPath.concat(file.d_name);
        dirPath = dirPathCopy;

        if (cv::VideoCapture(pathFileName).isOpened())
        {
            File fileStruct;

            std::string vtdiPath = ((std::string)pathFileName).substr(0, VariousUtils::rfind(pathFileName, '.')) + ".vtdi";
            if (std::filesystem::exists(vtdiPath) && validVTDI(vtdiPath))
            {
                if (pathFileName == path)
                {
                    filesIndex = i;
                }

                fileStruct.path = vtdiPath;
                fileStruct.type = FileType::VIDTRANS;
            }
            else
            {
                if (pathFileName == path)
                {
                    filesIndex = i;
                }
                
                fileStruct.path = pathFileName;
                fileStruct.type = FileType::VIDENC;
            }

            files.push_back(fileStruct);
        }
        else if (validVTDI(pathFileName))
        {
            if (pathFileName == path)
            {
                filesIndex = i;
            }
            
            File fileStruct;
            fileStruct.path = pathFileName;
            fileStruct.type = FileType::VIDTRANS;

            files.push_back(fileStruct);
        }
        else if (!cv::imread(pathFileName).empty())
        {
            if (pathFileName == path)
            {
                filesIndex = i;
            }
            
            File fileStruct;
            fileStruct.path = pathFileName;
            fileStruct.type = FileType::IMG;

            files.push_back(fileStruct);
        }

        i++;
    }
}

const bool MediaViewer::empty()
{
    return files.empty();
}

File* MediaViewer::current()
{
    if (files.empty()) return nullptr;

    return &files[filesIndex];
}

File* MediaViewer::next()
{
    if (files.empty()) return nullptr;

    filesIndex = (filesIndex + 1) % files.size();
    return &files[filesIndex];
}

File* MediaViewer::prev()
{
    if (files.empty()) return nullptr;

    filesIndex--;
    if (filesIndex < 0) filesIndex += files.size();
    return &files[filesIndex];
}

}
