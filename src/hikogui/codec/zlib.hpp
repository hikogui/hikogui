// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../file/file_view.hpp"
#include "../byte_string.hpp"
#include <cstddef>
#include <filesystem>

namespace hi::inline v1 {

bstring zlib_decompress(std::span<std::byte const> bytes, std::size_t max_size = 0x01000000);

inline bstring zlib_decompress(std::filesystem::path const &path, std::size_t max_size = 0x01000000)
{
    return zlib_decompress(as_span<std::byte const>(file_view(path)), max_size);
}

} // namespace hi::inline v1
