// Copyright 2019 Pokitec
// All rights reserved.

#include "thread.hpp"
#include "strings.hpp"
#include "Application.hpp"

#include <pthread.h>

namespace tt {

static std::atomic<uint32_t> thread_id_count = 0;

void set_thread_name(std::string_view name)
{
    tt_assert2(current_thread_id == 0, "set_thread_name() should be called exactly once per thread");

    current_thread_id = thread_id_count.fetch_add(1, std::memory_order::memory_order_relaxed) + 1;
    tt_assert2(current_thread_id != 0, "32 bit current_thread_id wrapped around");

    pthread_setname_np(name.data());
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
