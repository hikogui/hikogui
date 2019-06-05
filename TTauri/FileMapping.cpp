// Copyright 2019 Pokitec
// All rights reserved.

#include "FileMapping.hpp"
#include "required.hpp"
#include "logging.hpp"
#include "utils.hpp"

namespace TTauri {

std::map<std::filesystem::path, std::vector<std::weak_ptr<File>>> FileMapping::mappedFiles;

FileMapping::FileMapping(std::shared_ptr<File> const& file, size_t size) :
    file(file), size(size > 0 ? size : std::filesystem::file_size(file->path))
{
#ifdef WIN32
    DWORD protect;
    if (accessMode() >= AccessMode::RDWR) {
        protect = PAGE_READWRITE;
    }
    else if (accessMode() >= AccessMode::RDONLY) {
        protect = PAGE_READONLY;
    }
    else {
        BOOST_THROW_EXCEPTION(FileError("Illigal access mode WRONLY/0 when mapping file.") <<
            boost::errinfo_file_name(path().string())
        );
    }

    DWORD maximumSizeHigh = this->size >> 32;
    DWORD maximumSizeLow = this->size & 0xffffffff;

    if ((intrinsic = CreateFileMappingA(file->intrinsic, NULL, protect, maximumSizeHigh, maximumSizeLow, NULL)) == NULL) {
        BOOST_THROW_EXCEPTION(FileError(getLastErrorMessage()) <<
            boost::errinfo_file_name(path().string())
        );
    }
#endif
}

FileMapping::FileMapping(const std::filesystem::path & path, AccessMode accessMode, size_t size) :
    FileMapping(findOrCreateFile(path, accessMode), size) {}

FileMapping::~FileMapping()
{
    if (!CloseHandle(intrinsic)) {
        LOG_ERROR("Could not close file mapping object on file '%s'") % getLastErrorMessage();
    }
}

std::shared_ptr<File> FileMapping::findOrCreateFile(std::filesystem::path const& path, AccessMode accessMode)
{
    cleanup();

    let absolutePath = std::filesystem::absolute(path);

    auto& files = mappedFiles[absolutePath];
    for (let weak_file : files) {
        if (auto file = weak_file.lock()) {
            if (file->accessMode >= accessMode) {
                return file;
            }
        }
    }

    auto file = std::make_shared<File>(path, accessMode);
    files.push_back(file);
    return file;
}

void FileMapping::cleanup()
{
    for (auto& [key, files] : mappedFiles) {
        erase_if(files, [](auto x) {
            return x.expired();
        });
    }

    erase_if(mappedFiles, [](auto x) {
        return x.second.size() == 0;
    });
}

}