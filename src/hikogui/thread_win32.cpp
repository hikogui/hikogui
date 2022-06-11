// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "win32_headers.hpp"

#include "thread.hpp"
#include "strings.hpp"
#include "log.hpp"
#include "exception.hpp"
#include "unfair_mutex.hpp"
#include <mutex>
#include <string>
#include <unordered_map>
#include <format>

namespace hi::inline v1 {

static auto thread_names = std::unordered_map<thread_id, std::string>{};
static unfair_mutex thread_names_mutex = {};

void set_thread_name(std::string_view name) noexcept
{
    hilet wname = to_wstring(name);
    SetThreadDescription(GetCurrentThread(), wname.data());

    auto name_ = std::string{name};
    auto id = get_thread_id();

    hilet lock = std::scoped_lock(thread_names_mutex);
    thread_names.emplace(id, std::move(name_);
}

[[nodiscard]] std::string get_thread_name(thread_id id) noexcept
{
    hilet lock = std::scoped_lock(thread_names_mutex);
    hilet it = thread_names.find(id);
    if (it != thread_names.end()) {
        return *it;
    } else {
        return std::format("<{}>", id);
    }
}

static std::vector<bool> mask_int_to_vec(DWORD_PTR rhs) noexcept
{
    auto r = std::vector<bool>{};

    r.resize(64);
    for (std::size_t i = 0; i != r.size(); ++i) {
        r[i] = static_cast<bool>(rhs & (DWORD_PTR{1} << i));
    }

    return r;
}

static DWORD_PTR mask_vec_to_int(std::vector<bool> const &rhs) noexcept
{
    DWORD r = 0;
    for (std::size_t i = 0; i != rhs.size(); ++i) {
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
        hi_log_fatal("Could not get process affinity mask: {}", get_last_error_message());
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
    hi_axiom(index < 64);
    return index;
}

} // namespace hi::inline v1
