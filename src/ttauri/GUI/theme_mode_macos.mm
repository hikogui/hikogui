// Copyright 2020 Pokitec
// All rights reserved.

#include "theme_mode.hpp"
#include "../strings.hpp"
#include "../logger.hpp"

namespace tt {

[[nodiscard]] theme_mode readOStheme_mode() noexcept
{
    return theme_mode::Light;
}

}
