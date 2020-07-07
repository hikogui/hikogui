// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/GUI/KeyboardEvent.hpp"
#include "ttauri/GUI/globals.hpp"

namespace tt {

std::vector<string_ltag> const &KeyboardEvent::getCommands() const noexcept
{
    tt_assume(type == Type::Key);
    return keyboardBindings.translate(key);
}


}
