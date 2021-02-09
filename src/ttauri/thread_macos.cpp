// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "thread.hpp"
#include "strings.hpp"
#include "application.hpp"

#include <pthread.h>

namespace tt {

static std::atomic<uint32_t> thread_id_count = 0;

void set_thread_name(std::string_view name)
{
    pthread_setname_np(name.data());
}

bool is_main_thread()
{
    tt_axiom(application);
    return current_thread_id == application::global->main_thread_id;
}

void run_from_main_loop(std::function<void()> f)
{
    if (is_main_thread()) {
        return f();

    } else {
        tt_axiom(application);
        application->runFromMainLoop(f);
    }
}

}
