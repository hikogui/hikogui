// Copyright 2020 Pokitec
// All rights reserved.

#include "KeyboardEvent.hpp"
#include "KeyboardBindings.hpp"

namespace tt {

std::vector<command> const &KeyboardEvent::getCommands() const noexcept
{
    tt_assume(type == Type::Key);
    return keyboardBindings.translate(key);
}


}
