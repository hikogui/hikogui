// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/required.hpp"
#include "TTauri/Foundation/byte_string.hpp"
#include <optional>

namespace TTauri {

std::string base93_encode(bstring_view message) noexcept;

std::optional<bstring> base93_decode(std::string_view str, size_t &offset);

inline std::optional<bstring> base93_decode(std::string_view str)
{
    size_t offset = 0;
    return base93_decode(str, offset);
}

}
