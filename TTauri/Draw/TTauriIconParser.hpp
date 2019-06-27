// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <filesystem>
#include <gsl/gsl>

namespace TTauri::Draw {

struct Path;

Path parseTTauriIcon(gsl::span<std::byte> bytes);

Path parseTTauriIcon(std::filesystem::path& path);

}
