// Copyright 2019 Pokitec
// All rights reserved.

#include "thread.hpp"
#include "strings.hpp"
#include "Application.hpp"
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
    tt_assume(application);
    return std::this_thread::get_id() == application->mainThreadID;
}

void run_from_main_loop(std::function<void()> f)
{
    // Do not optimize by checking if this is called from the main thread
    // the function should be passed to the queue on the main loop.
    tt_assume(application);
    application->run_from_main_loop(f);
}

}