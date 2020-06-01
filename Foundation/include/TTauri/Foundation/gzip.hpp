// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/byte_string.hpp"
#include "TTauri/Foundation/FileView.hpp"
#include <cstddef>

namespace TTauri {

bstring gzip_decompress(gsl::span<std::byte const> bytes, ssize_t max_size=0x0100'0000);
bstring gzip_decompress(FileView const &view, ssize_t max_size=0x0100'0000);
bstring gzip_decompress(URL const &url, ssize_t max_size=0x0100'0000);

}