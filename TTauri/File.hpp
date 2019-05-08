// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#ifdef WIN32
#include <Windows.h>
#endif

#include <boost/exception/all.hpp>

#include <cstdint>
#include <filesystem>
#include <map>

namespace TTauri {

struct FileError : virtual boost::exception, virtual std::exception {
    std::string _what;

    FileError() : _what("unknown FileError") {}
    FileError(const std::string& what) : _what(what) {}

    const char* what() const noexcept override {
        return _what.data();
    }
};

struct AccessMode {
    uint64_t value;

    bool operator>=(uint64_t v) {
        return (value & v) > 0;
    }

    bool operator>=(AccessMode m) {
        return (value & m.value) > 0;
    }

    static constexpr uint64_t RDONLY = 0x1;
    static constexpr uint64_t WRONLY = 0x2;
    static constexpr uint64_t RDLOCK = 0x10;
    static constexpr uint64_t WRLOCK = 0x20;
    static constexpr uint64_t RDWR = RDONLY | WRONLY;
    static constexpr uint64_t CREAT = 0x100;
    static constexpr uint64_t EXCL = 0x200;
    static constexpr uint64_t TRUNC = 0x400;
    static constexpr uint64_t RANDOM_ACCESS = 0x1000;
    static constexpr uint64_t SEQUENTIAL = 0x2000;
    static constexpr uint64_t WRITE_THROUGH = 0x4000;
};

struct File {
    AccessMode accessMode;
    std::filesystem::path path;

#ifdef WIN32
    HANDLE intrinsic;
#endif

    File(std::filesystem::path const& path, AccessMode accessMode);
    ~File();
};


struct FileMapping {
    std::shared_ptr<File> file;
    size_t size;
#ifdef WIN32
    HANDLE intrinsic;
#endif

    FileMapping(std::shared_ptr<File> const &file, size_t size);
    FileMapping(std::filesystem::path const &path, AccessMode accessMode, size_t size);
    ~FileMapping();

    AccessMode accessMode() { return file->accessMode; }
    std::filesystem::path path() { return file->path; }

    static std::shared_ptr<File> findOrCreateFile(std::filesystem::path const &path, AccessMode accessMode);
    static void cleanup();
    static std::map<std::filesystem::path, std::vector<std::weak_ptr<File>>> mappedFiles;
};


struct FileView {
    std::shared_ptr<FileMapping> fileMappingObject;
    size_t offset;
    size_t size;
    void *data;

    FileView(std::shared_ptr<FileMapping> const& mappingObject, size_t offset, size_t size);
    FileView(std::filesystem::path const& path, AccessMode accessMode, size_t offset, size_t size);
    ~FileView();

    AccessMode accessMode() { return fileMappingObject->accessMode(); }
    std::filesystem::path path() { return fileMappingObject->path(); }

    void flush(void *base, size_t size);

    static std::shared_ptr<FileMapping> findOrCreateFileMappingObject(std::filesystem::path const &path, AccessMode accessMode, size_t size);
    static void cleanup();
    static std::map<std::filesystem::path, std::vector<std::weak_ptr<FileMapping>>> mappedFileObjects;
};

}