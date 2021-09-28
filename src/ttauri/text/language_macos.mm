// Copyright 2020 Pokitec
// All rights reserved.

#include "language.hpp"
#include "../required.hpp"
#include "../log.hpp"

namespace tt {

std::vector<std::string> language::get_preferred_language_tags() noexcept
{
    return std::vector<std::string>{"en-US"};
}

}
