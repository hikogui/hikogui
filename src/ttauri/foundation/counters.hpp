// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/foundation/wfree_unordered_map.hpp"
#include <nonstd/span>
#include <typeinfo>
#include <typeindex>

namespace tt {

constexpr int MAX_NR_COUNTERS = 1024;

struct counter_map_value_type {
    std::atomic<int64_t> *counter;
    int64_t previous_value;
};

using counter_map_type = wfree_unordered_map<std::type_index,counter_map_value_type,MAX_NR_COUNTERS>;

// To reduce number of executed instruction this is a global varable.
// The wfree_unordered_map does not need to be initialized.
inline counter_map_type counter_map;

template<typename Tag>
struct counter_functor {
    // Make sure non of the counters are false sharing cache-lines.
    alignas(cache_line_size) inline static std::atomic<int64_t> counter = 0;

    tt_no_inline void add_to_map() const noexcept {
        counter_map.insert(std::type_index(typeid(Tag)), counter_map_value_type{&counter, 0});
    }

    int64_t increment() const noexcept {
        ttlet value = counter.fetch_add(1, std::memory_order_relaxed);

        if (tt_unlikely(value == 0)) {
            add_to_map();
        }

        return value + 1;
    }

    int64_t read() const noexcept {
        return counter.load(std::memory_order_relaxed);
    }

    // Don't implement readAndSet, a set to zero would cause the counters to be reinserted.
};

template<typename Tag>
inline int64_t increment_counter() noexcept 
{
    return counter_functor<Tag>{}.increment();
}

template<typename Tag>
inline int64_t read_counter() noexcept
{
    return counter_functor<Tag>{}.read();
}

/*!
 * \return The current count, count since last read.
 */
inline std::pair<int64_t, int64_t> read_counter(std::type_index tag) noexcept
{
    auto &item = counter_map[tag];

    ttlet * const count_ptr = item.counter;
    ttlet count = count_ptr != nullptr ? item.counter->load(std::memory_order_relaxed) : 0;
    ttlet count_since_last_read = count - item.previous_value;
    item.previous_value = count;
    return {count, count_since_last_read};
}

}
