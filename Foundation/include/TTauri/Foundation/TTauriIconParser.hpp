// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <nonstd/span>

namespace TTauri {
class URL;
}

namespace TTauri {

struct Path;

Path parseTTauriIcon(nonstd::span<std::byte const> bytes);

}
