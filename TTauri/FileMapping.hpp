// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "File.hpp"
#include <memory>
#include <unordered_map>

namespace TTauri {

struct FileMapping {
    std::shared_ptr<File> file;
    size_t size;
#ifdef WIN32
    HANDLE intrinsic;
#endif

    FileMapping(std::shared_ptr<File> const& file, size_t size);
    FileMapping(URL const& path, AccessMode accessMode, size_t size);
    ~FileMapping();

    FileMapping(FileMapping const &other) = delete;
    FileMapping(FileMapping &&other) = delete;
    FileMapping &operator=(FileMapping const &other) = delete;
    FileMapping &operator=(FileMapping &&other) = delete;

    AccessMode accessMode() { return file->accessMode; }
    URL const &location() { return file->location; }

    static std::shared_ptr<File> findOrCreateFile(URL const& path, AccessMode accessMode);
    static void cleanup();
    static inline std::unordered_map<URL, std::vector<std::weak_ptr<File>>> mappedFiles;
};

}