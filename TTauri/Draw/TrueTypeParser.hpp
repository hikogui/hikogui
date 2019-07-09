// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <gsl/gsl>

namespace TTauri::Draw {

struct Font;

Font parseTrueTypeFile(gsl::span<std::byte const> bytes);

}