// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/GUI/ThemeMode.hpp"
#include "ttauri/foundation/strings.hpp"
#include "ttauri/foundation/logger.hpp"

namespace tt {

[[nodiscard]] ThemeMode readOSThemeMode() noexcept
{
    return ThemeMode::Light;
}

}
