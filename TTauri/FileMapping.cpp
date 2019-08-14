// Copyright 2019 Pokitec
// All rights reserved.

#include "FileMapping.hpp"
#include "required.hpp"
#include "logging.hpp"
#include "utils.hpp"
#include <mutex>

namespace TTauri {

FileMapping::FileMapping(std::shared_ptr<File> const& file, size_t size) :
    file(file), size(size > 0 ? size : file_size(file->location))
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
        TTAURI_THROW(io_error("Illigal access mode WRONLY/0 when mapping file.")
            << error_info("url", location())
        );
    }

    DWORD maximumSizeHigh = this->size >> 32;
    DWORD maximumSizeLow = this->size & 0xffffffff;

    if ((intrinsic = CreateFileMappingA(file->intrinsic, NULL, protect, maximumSizeHigh, maximumSizeLow, nullptr)) == nullptr) {
        TTAURI_THROW(io_error("Could not create file mapping")
            << error_info("error-message", getLastErrorMessage())
            << error_info("url", location())
        );
    }
#endif
}

FileMapping::FileMapping(URL const &location, AccessMode accessMode, size_t size) :
    FileMapping(findOrCreateFile(location, accessMode), size) {}

FileMapping::~FileMapping()
{
    if (!CloseHandle(intrinsic)) {
        LOG_ERROR("Could not close file mapping object on file '%s'", getLastErrorMessage());
    }
}

std::shared_ptr<File> FileMapping::findOrCreateFile(URL const& location, AccessMode accessMode)
{
    static std::mutex mutex;
    static std::unordered_map<URL, std::vector<std::weak_ptr<File>>> mappedFiles;

    let lock = std::scoped_lock(mutex);

    cleanupWeakPointers(mappedFiles);

    // We want files to be freshly created if it did not exist before.
    auto& files = mappedFiles[location];
    for (let weak_file : files) {
        if (auto file = weak_file.lock()) {
            if (file->accessMode >= accessMode) {
                return file;
            }
        }
    }

    auto file = std::make_shared<File>(location, accessMode);
    files.push_back(file);
    return file;
}

}