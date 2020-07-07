// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "language.hpp"

namespace tt {

std::vector<std::string> language::get_preferred_language_tags() noexcept
{
    return std::vector<std::string>{"en-US"};
}

}
