
#pragma once

#include "byte_string.hpp"
#include <span>
#include <string>
#include <string_view>

namespace tt {

byte_string base64_decode(std::string_view encoded_text) noexcept;

std::string base64_encode(nonstd::span<std::byte const> binary_data) noexcept;

}

