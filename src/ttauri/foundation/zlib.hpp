// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/byte_string.hpp"
#include "TTauri/Foundation/FileView.hpp"
#include <cstddef>

namespace tt {

bstring zlib_decompress(nonstd::span<std::byte const> bytes, ssize_t max_size=0x0100'0000);

inline bstring zlib_decompress(URL const &url, ssize_t max_size=0x0100'0000) {
    return zlib_decompress(FileView(url), max_size);
}

}