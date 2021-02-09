// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "thread.hpp"
#include "strings.hpp"
#include "application.hpp"
#include "logger.hpp"

#include <Windows.h>
#include <Synchapi.h>

namespace tt {

void set_thread_name(std::string_view name)
{
    ttlet wname = to_wstring(name);
    SetThreadDescription(GetCurrentThread(), wname.data());
}

bool is_main_thread()
{
    tt_axiom(application::global);
    return current_thread_id() == application::global->main_thread_id;
}

void run_from_main_loop(std::function<void()> f)
{
    // Do not optimize by checking if this is called from the main thread
    // the function should be passed to the queue on the main loop.
    tt_axiom(application::global);
    application::global->run_from_main_loop(f);
}

}