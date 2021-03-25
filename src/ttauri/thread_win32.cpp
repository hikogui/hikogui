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

[[nodiscard]] uint64_t process_affinity_mask() noexcept
{
    DWORD_PTR process_mask;
    DWORD_PTR system_mask;

    auto process_handle = GetCurrentProcess();

    auto r = GetProcessAffinityMask(process_handle, &process_mask, &system_mask);

    return narrow_cast<uint64_t>(process_mask);
}

uint64_t set_thread_affinity_mask(uint64_t mask) noexcept
{
    auto thread_handle = GetCurrentThread();

    auto r = SetThreadAffinityMask(thread_handle, narrow_cast<DWORD_PTR>(mask));
    return narrow_cast<uint64_t>(r);
}

[[nodiscard]] size_t current_processor() noexcept
{
    ttlet index = GetCurrentProcessorNumber();
    tt_axiom(index < 64);
    return index;
}



}
