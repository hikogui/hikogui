// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/foundation/required.hpp"
#include "ttauri/foundation/byte_string.hpp"
#include <optional>

namespace tt {

std::string base93_encode(bstring_view message) noexcept;

std::optional<bstring> base93_decode(std::string_view str, size_t &offset);

inline std::optional<bstring> base93_decode(std::string_view str)
{
    size_t offset = 0;
    return base93_decode(str, offset);
}

}
