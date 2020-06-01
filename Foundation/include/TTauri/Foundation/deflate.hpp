// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/byte_string.hpp"
#include <gsl/gsl>

namespace TTauri {

bstring deflate_decompress(gsl::span<std::byte const> bytes, ssize_t &offset, ssize_t max_size=0x0100'0000);

}