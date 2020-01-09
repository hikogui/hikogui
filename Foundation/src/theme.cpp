// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/JSON.hpp"
#include "TTauri/Foundation/theme.hpp"

namespace TTauri {


[[nodiscard]] theme parse_theme(URL const &url) noexcept
{
    let data = parseJSON(url);
}

}