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

    auto unfilteredFiles = VariousUtils::getFilesInDir(dirPath);
    sort(unfilteredFiles.begin(), unfilteredFiles.end(), [](dirent d1, dirent d2){return (std::string)d1.d_name < (std::string)d2.d_name;});
    for (const auto file : unfilteredFiles)
    {
        std::string pathFileName = dirPath / file.d_name;

        if (!cv::imread(pathFileName).empty())
        {
            if (checkForFile && pathFileName == path)
            {
                filesIndex = files.size();
            }
            
            File fileStruct;
            fileStruct.path = pathFileName;
            fileStruct.type = FileType::IMG;

            files.push_back(fileStruct);
        }
        else if (cv::VideoCapture(pathFileName).isOpened())
        {
            File fileStruct;

            std::string vtdiPath = ((std::string)pathFileName).substr(0, VariousUtils::rfind(pathFileName, '.')) + ".vtdi";
            if (std::filesystem::exists(vtdiPath) && validVTDI(vtdiPath))
            {
                if (checkForFile && pathFileName == path)
                {
                    filesIndex = files.size();
                }

                fileStruct.path = vtdiPath;
                fileStruct.type = FileType::VIDTRANS;
            }
            else
            {
                if (checkForFile && pathFileName == path)
                {
                    filesIndex = files.size();
                }
                
                fileStruct.path = pathFileName;
                fileStruct.type = FileType::VIDENC;
            }

            files.push_back(fileStruct);
        }
        else if (validVTDI(pathFileName))
        {
            // VTDI file might have already been added in a previous mp4 check. Make sure it's not a dupe.
            if (!std::none_of(files.begin(), files.end(), [&pathFileName](File d){return std::strcmp(d.path.c_str(), pathFileName.c_str()) == 0;}))
            {
                continue;
            }
            if (checkForFile && pathFileName == path)
            {
                filesIndex = files.size();
            }
            
            File fileStruct;
            fileStruct.path = pathFileName;
            fileStruct.type = FileType::VIDTRANS;

            files.push_back(fileStruct);
        }
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

void MediaViewer::view(std::array<int, 2> maxDims)
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
    }
}

}
