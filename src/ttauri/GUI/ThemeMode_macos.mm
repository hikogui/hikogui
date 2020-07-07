// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/GUI/ThemeMode.hpp"
#include "ttauri/strings.hpp"
#include "ttauri/logger.hpp"

namespace tt {

[[nodiscard]] ThemeMode readOSThemeMode() noexcept
{
    return ThemeMode::Light;
}

}
