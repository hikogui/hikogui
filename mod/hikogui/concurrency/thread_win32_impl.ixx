// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"
#include "../win32_headers.hpp"


#include <intrin.h>
#include <mutex>
#include <string>
#include <unordered_map>

export module hikogui_concurrency_thread : impl;
import : intf;
import hikogui_char_maps;
import hikogui_concurrency_unfair_mutex;
import hikogui_utility;

export namespace hi::inline v1 {

[[nodiscard]] thread_id current_thread_id() noexcept
{
    // Thread IDs on Win32 are guaranteed to be not zero.
    constexpr uint64_t NT_TIB_CurrentThreadID = 0x48;
    return __readgsdword(NT_TIB_CurrentThreadID);
}

void set_thread_name(std::string_view name) noexcept
{
    hilet wname = hi::to_wstring(name);
    SetThreadDescription(GetCurrentThread(), wname.c_str());

    hilet lock = std::scoped_lock(detail::thread_names_mutex);
    detail::thread_names.emplace(current_thread_id(), std::string{name});
}

 std::vector<bool> mask_int_to_vec(DWORD_PTR rhs) noexcept
{
    auto r = std::vector<bool>{};

    r.resize(64);
    for (std::size_t i = 0; i != r.size(); ++i) {
        r[i] = to_bool(rhs & (DWORD_PTR{1} << i));
    }

    return r;
}

 DWORD_PTR mask_vec_to_int(std::vector<bool> const &rhs) noexcept
{
    DWORD r = 0;
    for (std::size_t i = 0; i != rhs.size(); ++i) {
        r |= rhs[i] ? (DWORD{1} << i) : 0;
    }
    return r;
}

[[nodiscard]] std::vector<bool> process_affinity_mask()
{
    DWORD_PTR process_mask;
    DWORD_PTR system_mask;

    auto process_handle = GetCurrentProcess();

    if (not GetProcessAffinityMask(process_handle, &process_mask, &system_mask)) {
        throw os_error(std::format("Could not get process affinity mask.", get_last_error_message()));
    }

    return mask_int_to_vec(process_mask);
}

std::vector<bool> set_thread_affinity_mask(std::vector<bool> const &mask)
{
    hilet mask_ = mask_vec_to_int(mask);

    hilet thread_handle = GetCurrentThread();

    hilet old_mask = SetThreadAffinityMask(thread_handle, mask_);
    if (old_mask == 0) {
        throw os_error(std::format("Could not set the thread affinity. '{}'", get_last_error_message()));
    }

    return mask_int_to_vec(old_mask);
}

[[nodiscard]] std::size_t current_cpu_id() noexcept
{
    hilet index = GetCurrentProcessorNumber();
    hi_assert(index < 64);
    return index;
}

} // namespace hi::inline v1
