// Copyright 2020 Pokitec
// All rights reserved.

#include "ThemeMode.hpp"
#include "../strings.hpp"
#include "../logger.hpp"

namespace tt {

[[nodiscard]] ThemeMode readOSThemeMode() noexcept
{
    return ThemeMode::Light;
}

}
