// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Font.hpp"

#include <filesystem>
#include <gsl/gsl>

namespace TTauri::Draw {


Font parseTrueTypeFile(gsl::span<std::byte> bytes);
Font parseTrueTypeFile(std::filesystem::path& path);

}