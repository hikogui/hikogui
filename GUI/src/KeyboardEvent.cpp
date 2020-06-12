// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/GUI/KeyboardEvent.hpp"
#include "TTauri/GUI/globals.hpp"

namespace tt {

std::vector<string_ltag> const &KeyboardEvent::getCommands() const noexcept
{
    ttauri_assume(type == Type::Key);
    return keyboardBindings.translate(key);
}


}