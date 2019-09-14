// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/File.hpp"
#include <memory>
#include <unordered_map>

namespace TTauri {

struct FileMapping {
    std::shared_ptr<File> file;
    size_t size;
    void *mapHandle;

    FileMapping(std::shared_ptr<File> const& file, size_t size);
    FileMapping(URL const& path, AccessMode accessMode, size_t size);
    ~FileMapping();

    FileMapping(FileMapping const &other) = delete;
    FileMapping(FileMapping &&other) = delete;
    FileMapping &operator=(FileMapping const &other) = delete;
    FileMapping &operator=(FileMapping &&other) = delete;

    AccessMode accessMode() const noexcept{ return file->accessMode; }
    URL const &location() const noexcept { return file->location; }

    static std::shared_ptr<File> findOrCreateFile(URL const& path, AccessMode accessMode);
};

}
