// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "FileMapping.hpp"
#include <gsl/gsl>

namespace TTauri {

struct FileView {
    std::shared_ptr<FileMapping> fileMappingObject;
    size_t offset;
    gsl::span<std::byte> bytes;

    FileView(std::shared_ptr<FileMapping> const& mappingObject, size_t offset, size_t size);
    FileView(URL const &location, AccessMode accessMode=AccessMode::RDONLY, size_t offset=0, size_t size=0);
    ~FileView();

    FileView(FileView const &other) = delete;
    FileView(FileView &&other);
    FileView &operator=(FileView const &other) = delete;
    FileView &operator=(FileView &&other) = delete;

    AccessMode accessMode() { return fileMappingObject->accessMode(); }
    URL const &location() { return fileMappingObject->location(); }

    void flush(void* base, size_t size);

    static std::shared_ptr<FileMapping> findOrCreateFileMappingObject(URL const& path, AccessMode accessMode, size_t size);
    static void cleanup();
    static inline std::unordered_map<URL, std::vector<std::weak_ptr<FileMapping>>> mappedFileObjects;
};

}