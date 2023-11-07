// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <cstddef>
#include <filesystem>

export module hikogui_codec_zlib;
import hikogui_codec_inflate;
import hikogui_container;
import hikogui_file;
import hikogui_parser;

export namespace hi { inline namespace v1 {

[[nodiscard]] bstring zlib_decompress(std::span<std::byte const> bytes, std::size_t max_size)
{
    struct zlib_header {
        uint8_t CMF;
        uint8_t FLG;
    };

    auto offset = 0_uz;

    hilet header = make_placement_ptr<zlib_header>(bytes, offset);

    hilet header_chksum = header->CMF * 256 + header->FLG;
    hi_check(header_chksum % 31 == 0, "zlib header checksum failed.");

    hi_check((header->CMF & 0xf) == 8, "zlib compression method must be 8");
    hi_check(((header->CMF >> 4) & 0xf) <= 7, "zlib LZ77 window too large");
    hi_check((header->FLG & 0x20) == 0, "zlib must not use a preset dictionary");

    if (header->FLG & 0x20) {
        [[maybe_unused]] auto FDICT = make_placement_ptr<big_uint32_buf_t>(bytes, offset);
    }

    auto r = inflate(bytes, offset, max_size);

    [[maybe_unused]] auto ADLER32 = make_placement_ptr<big_uint32_buf_t>(bytes, offset);

    return r;
}

[[nodiscard]] bstring zlib_decompress(std::filesystem::path const& path, std::size_t max_size = 0x01000000)
{
    return zlib_decompress(as_span<std::byte const>(file_view(path)), max_size);
}

}} // namespace hi::v1
