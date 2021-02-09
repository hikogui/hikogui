// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../URL.hpp"
#include "../byte_string.hpp"
#include "../file_view.hpp"
#include <cstddef>

namespace tt {

bstring zlib_decompress(std::span<std::byte const> bytes, ssize_t max_size=0x01000000);

inline bstring zlib_decompress(URL const &url, ssize_t max_size=0x01000000) {
    return zlib_decompress(file_view(url), max_size);
}

}
