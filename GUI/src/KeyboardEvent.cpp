// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/GUI/KeyboardEvent.hpp"
#include "TTauri/GUI/globals.hpp"

namespace TTauri::GUI {

string_ltag KeyboardEvent::getCommand(string_tag context) const noexcept
{
    ttauri_assume(type == Type::Key);
    ttauri_assume(GUI_globals != nullptr);
    return GUI_globals->keyboard_bindings.translate(context, key);
}


}