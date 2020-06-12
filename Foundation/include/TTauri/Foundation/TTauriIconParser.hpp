// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <nonstd/span>

namespace tt {
class URL;
}

namespace tt {

struct Path;

Path parseTTauriIcon(nonstd::span<std::byte const> bytes);

}
