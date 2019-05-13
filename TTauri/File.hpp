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

    AccessMode(uint64_t v) : value(v) {}

    //bool operator>=(uint64_t v) {
    //    return (value & v) == v;
    //}

    bool operator>=(AccessMode m) {
        return (value & m.value) == m.value;
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

}