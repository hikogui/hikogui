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
    FileView(std::filesystem::path const& path, AccessMode accessMode=AccessMode::RDONLY, size_t offset=0, size_t size=0);
    ~FileView();

    AccessMode accessMode() { return fileMappingObject->accessMode(); }
    std::filesystem::path path() { return fileMappingObject->path(); }

    void flush(void* base, size_t size);

    static std::shared_ptr<FileMapping> findOrCreateFileMappingObject(std::filesystem::path const& path, AccessMode accessMode, size_t size);
    static void cleanup();
    static std::map<std::filesystem::path, std::vector<std::weak_ptr<FileMapping>>> mappedFileObjects;
};

}