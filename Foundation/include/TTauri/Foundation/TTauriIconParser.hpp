// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <gsl/gsl>

namespace TTauri {
class URL;
}

namespace TTauri {

struct Path;

Path parseTTauriIcon(gsl::span<std::byte const> bytes);

}
