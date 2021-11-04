// Copyright 2020 Pokitec
// All rights reserved.

#include "theme_mode.hpp"
#include "../strings.hpp"
#include "../log.hpp"

namespace tt::inline v1 {

[[nodiscard]] theme_mode readOStheme_mode() noexcept
{
    return theme_mode::Light;
}

}
