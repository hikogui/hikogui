// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <gsl/gsl>

namespace TTauri {
struct URL;
}

namespace TTauri::Draw {

struct Path;

Path parseTTauriIcon(gsl::span<std::byte const> bytes);

}
