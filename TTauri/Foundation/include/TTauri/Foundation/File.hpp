// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/URL.hpp"

#ifdef WIN32
#include <Windows.h>
#endif

#include <cstdint>
#include <map>

namespace TTauri {


struct AccessMode {
    uint64_t value;

    AccessMode(uint64_t v) noexcept : value(v) {}

    bool operator>=(AccessMode m) noexcept {
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
    URL location;

#ifdef WIN32
    HANDLE intrinsic;
#endif

    File(URL const& location, AccessMode accessMode);
    ~File() noexcept;

    File(File const &other) = delete;
    File(File &&other) = delete;
    File &operator=(File const &other) = delete;
    File &operator=(File &&other) = delete;

    void close();
};

}
