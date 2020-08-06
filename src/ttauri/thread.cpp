// Copyright 2019 Pokitec
// All rights reserved.

#include "thread.hpp"
#include "strings.hpp"
#include "Application.hpp"

#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include <Windows.h>
#elif __has_include(<pthread.h>)
#include <pthread.h>
#endif

namespace tt {

static std::atomic<uint32_t> thread_id_count = 0;

void set_thread_name(std::string_view name)
{
    tt_assert(current_thread_id == 0);

#if  TT_OPERATING_SYSTEM == TT_OS_WINDOWS
    current_thread_id = GetCurrentThreadID();
    tt_assert(current_thread_id != 0, "According to Microsoft no thread can have the value zero");

    ttlet wname = to_wstring(name);
    SetThreadDescription(GetCurrentThread(), wname.data());
#elif  TT_OPERATING_SYSTEM == TT_OS_MACOS
    current_thread_id = thread_id_count.fetch_add(1, std::memory_order::memory_order_relaxed) + 1;
    tt_assert(current_thread_id != 0, "32 bit current_thread_id wrapped around");
    pthread_setname_np(name.data());
#elif  TT_OPERATING_SYSTEM == TT_OS_LINUX
    current_thread_id = thread_id_count.fetch_add(1, std::memory_order::memory_order_relaxed) + 1;
    tt_assert(current_thread_id != 0, "32 bit current_thread_id wrapped around");
    pthread_setname_np(pthread_self(), name.data());
#endif
}

bool is_main_thread()
{
    tt_assume(application);
    return std::this_thread::get_id() == application->mainThreadID;
}

void run_on_main_thread(std::function<void()> f)
{
    if (is_main_thread()) {
        return f();

    } else {
        tt_assume(application);
        application->runOnMainThread(f);
    }
}

}
