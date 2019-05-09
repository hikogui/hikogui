// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "File.hpp"
#include <memory>

namespace TTauri {

struct FileMapping {
    std::shared_ptr<File> file;
    size_t size;
#ifdef WIN32
    HANDLE intrinsic;
#endif

    FileMapping(std::shared_ptr<File> const& file, size_t size);
    FileMapping(std::filesystem::path const& path, AccessMode accessMode, size_t size);
    ~FileMapping();

    AccessMode accessMode() { return file->accessMode; }
    std::filesystem::path path() { return file->path; }

    static std::shared_ptr<File> findOrCreateFile(std::filesystem::path const& path, AccessMode accessMode);
    static void cleanup();
    static std::map<std::filesystem::path, std::vector<std::weak_ptr<File>>> mappedFiles;
};

}