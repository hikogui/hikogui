// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../URL.hpp"
#include "../byte_string.hpp"
#include "../ResourceView.hpp"
#include <cstddef>

namespace tt {

bstring gzip_decompress(nonstd::span<std::byte const> bytes, ssize_t max_size=0x0100'0000);

inline bstring gzip_decompress(URL const &url, ssize_t max_size=0x0100'0000) {
    return gzip_decompress(*url.loadView(), max_size);
}

}