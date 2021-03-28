// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "thread.hpp"
#include "strings.hpp"
#include "application.hpp"
#include "logger.hpp"
#include "exception.hpp"
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

static std::vector<bool> mask_int_to_vec(DWORD_PTR rhs) noexcept
{
    auto r = std::vector<bool>{};

    r.resize(64);
    for (size_t i = 0; i != r.size(); ++i) {
        r[i] = static_cast<bool>(rhs & (DWORD_PTR{1} << i));
    }

    return r;
}

static DWORD_PTR mask_vec_to_int(std::vector<bool> const &rhs) noexcept
{
    DWORD r = 0;
    for (size_t i = 0; i != rhs.size(); ++i) {
        r |= rhs[i] ? (DWORD{1} << i) : 0;
    }
    return r;
}

[[nodiscard]] std::vector<bool> process_affinity_mask() noexcept
{
    DWORD_PTR process_mask;
    DWORD_PTR system_mask;

    auto process_handle = GetCurrentProcess();

    if (!GetProcessAffinityMask(process_handle, &process_mask, &system_mask)) {
        tt_log_fatal("Could not get process affinity mask: {}", get_last_error_message());
    }

    return mask_int_to_vec(process_mask);
}

std::vector<bool> set_thread_affinity_mask(std::vector<bool> const &mask)
{
    ttlet mask_ = mask_vec_to_int(mask);

    ttlet thread_handle = GetCurrentThread();

    ttlet old_mask = SetThreadAffinityMask(thread_handle, mask_);
    if (old_mask == 0) {
        throw os_error("Could not set the thread affinity. '{}'", get_last_error_message());
    }

    return mask_int_to_vec(old_mask);
}

[[nodiscard]] size_t current_processor() noexcept
{
    ttlet index = GetCurrentProcessorNumber();
    tt_axiom(index < 64);
    return index;
}



}
