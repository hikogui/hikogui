// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/GUI/ThemeMode.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/logger.hpp"

namespace tt {

[[nodiscard]] ThemeMode readOSThemeMode() noexcept
{
    return ThemeMode::Light;
}

}
