// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../byte_string.hpp"
#include <span>
#include <string>
#include <string_view>

namespace tt {

bstring decode_base64(std::string_view encoded_text);

std::string encode_base64(std::span<std::byte const> binary_data) noexcept;

inline std::string encode_base64(bstring const &binary_data) noexcept
{
    return encode_base64(std::span(binary_data));
}

}

