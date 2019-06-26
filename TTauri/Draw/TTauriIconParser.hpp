// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <filesystem>
#include <gsl/gsl>

namespace TTauri::Draw {

struct Drawing;

Drawing parseTTauriIcon(gsl::span<std::byte> bytes);

Drawing parseTTauriIcon(std::filesystem::path& path);

}
