// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/thread.hpp"
#include "ttauri/strings.hpp"
#include "ttauri/globals.hpp"

#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include <Windows.h>
#elif __has_include(<pthread.h>)
#include <pthread.h>
#endif

namespace tt {

void set_thread_name(std::string_view name)
{
#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
    ttlet wname = to_wstring(name);
    SetThreadDescription(GetCurrentThread(), wname.data());
#elif  TT_OPERATING_SYSTEM == TT_OS_MACOS
    pthread_setname_np(name.data());
#elif  TT_OPERATING_SYSTEM == TT_OS_LINUX
    pthread_setname_np(pthread_self(), name.data());
#endif
}

bool is_main_thread()
{
    return std::this_thread::get_id() == mainThreadID;
}

void run_on_main_thread(std::function<void()> f)
{
    if (is_main_thread()) {
        return f();

    } else if (mainThreadRunner) {
        mainThreadRunner(f);

    } else {
        // We could not run the thread on the main thread.
        tt_no_default;
    }
}

}