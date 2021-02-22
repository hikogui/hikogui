// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "wfree_unordered_map.hpp"
#include "os_detect.hpp"
#include "fixed_string.hpp"
#include "statistics.hpp"
#include <span>
#include <typeinfo>
#include <typeindex>
#include <string>

namespace tt {

constexpr int MAX_NR_COUNTERS = 1024;

struct counter_map_value_type {
    std::atomic<int64_t> *counter;
    int64_t previous_value;
};

using counter_map_type = wfree_unordered_map<std::string, counter_map_value_type, MAX_NR_COUNTERS>;

// To reduce number of executed instruction this is a global variable.
// The wfree_unordered_map does not need to be initialized.
inline counter_map_type counter_map;

template<basic_fixed_string Tag>
struct counter_functor {
    // Make sure non of the counters are false sharing cache-lines.
    alignas(hardware_destructive_interference_size) inline static std::atomic<int64_t> counter = 0;

    tt_no_inline void add_to_map() const noexcept
    {
        counter_map.insert(Tag, counter_map_value_type{&counter, 0});
        statistics_start();
    }

    int64_t increment() const noexcept
    {
        ttlet value = counter.fetch_add(1, std::memory_order::relaxed);

        if (value == 0) {
            [[unlikely]] add_to_map();
        }

        return value + 1;
    }

    [[nodiscard]] int64_t read() const noexcept
    {
        return counter.load(std::memory_order::relaxed);
    }

    // Don't implement readAndSet, a set to zero would cause the counters to be reinserted.
};

template<basic_fixed_string Tag>
inline int64_t increment_counter() noexcept
{
    return counter_functor<Tag>{}.increment();
}

template<basic_fixed_string Tag>
[[nodiscard]] inline int64_t read_counter() noexcept
{
    return counter_functor<Tag>{}.read();
}

/*!
 * \return The current count, count since last read.
 */
[[nodiscard]] inline std::pair<int64_t, int64_t> read_counter(std::string tag) noexcept
{
    auto &item = counter_map[tag];

    ttlet *const count_ptr = item.counter;
    ttlet count = count_ptr != nullptr ? item.counter->load(std::memory_order::relaxed) : 0;
    ttlet count_since_last_read = count - item.previous_value;
    item.previous_value = count;
    return {count, count_since_last_read};
}

} // namespace tt
