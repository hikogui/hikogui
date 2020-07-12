// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../byte_string.hpp"
#include <span>
#include <string>
#include <string_view>

namespace tt {

bstring base64_decode(std::string_view encoded_text);

std::string base64_encode(nonstd::span<std::byte const> binary_data) noexcept;

inline std::string base64_encode(bstring const &binary_data) noexcept
{
    return base64_encode(nonstd::span(binary_data));
}

}

