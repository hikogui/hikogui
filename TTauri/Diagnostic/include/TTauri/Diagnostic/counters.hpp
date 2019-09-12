// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/required.hpp"
#include "TTauri/Required/string_tag.hpp"
#include "TTauri/Required/wfree_unordered_map.hpp"
#include <gsl/gsl>

namespace TTauri {

constexpr int MAX_NR_COUNTERS = 1024;

struct counter_map_value_type {
    std::atomic<int64_t> *counter;
    int64_t previous_value;
};

using counter_map_type = wfree_unordered_map<string_tag,counter_map_value_type,MAX_NR_COUNTERS>;

// To reduce number of executed instruction this is a global varable.
// The wfree_unordered_map does not need to be initialized.
inline counter_map_type counter_map;

template<string_tag TAG>
struct counter_functor {
    // Make sure non of the counters are false sharing cache-lines.
    alignas(cache_line_size) inline static std::atomic<int64_t> counter = 0;

    no_inline void add_to_map() const noexcept no_inline_attr {
        counter_map.insert(TAG, counter_map_value_type{&counter, 0});
    }

    int64_t increment() const noexcept {
        let value = counter.fetch_add(1, std::memory_order_relaxed);

        if (ttauri_unlikely(value == 0)) {
            add_to_map();
        }

        return value + 1;
    }

    int64_t read() const noexcept {
        return counter.load(std::memory_order_relaxed);
    }

    // Don't implement readAndSet, a set to zero would cause the counters to be reinserted.
};

template<string_tag TAG>
inline int64_t increment_counter() noexcept 
{
    return counter_functor<TAG>{}.increment();
}

template<string_tag TAG>
inline int64_t read_counter() noexcept
{
    return counter_functor<TAG>{}.read();
}

/*!
 * \return The current count, count since last read.
 */
inline std::pair<int64_t, int64_t> read_counter(string_tag tag) noexcept
{
    auto &item = counter_map[tag];

    let * const count_ptr = item.counter;
    let count = count_ptr != nullptr ? item.counter->load(std::memory_order_relaxed) : 0;
    let count_since_last_read = count - item.previous_value;
    item.previous_value = count;
    return {count, count_since_last_read};
}

inline std::pair<int64_t, int64_t> read_counter(std::string_view name) noexcept
{
    return read_counter(string_to_tag(name));
}

}
