// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "debugger.hpp"
#include "dialog.hpp"
#include "console.hpp"
#include "log.hpp"
#include <exception>

namespace tt::inline v1 {

[[noreturn]] void debugger_abort(std::string const &message) noexcept
{
    log_global.flush();

    if (debugger_is_present()) {
        console_output(message);
        tt_debugger_break();
    } else {
        dialog_ok("Aborting", message);
    }

    std::terminate();
}

}
