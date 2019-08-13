// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "wfree_unordered_map.hpp"

namespace TTauri {

constexpr int MAX_NR_COUNTERS = 1000;

template<char... chars>
inline auto &get_counter_ref(string_tag<chars...>) noexcept
{
    static std::atomic<uint64_t> counter = 0;
    return counter;
}

inline auto &get_counter_map() noexcept
{
    static wfree_unordered_map<MAX_NR_COUNTERS, std::string, std::atomic<uint64_t> *> counter_map = {};
    return counter_map;
}

template<char... chars>
inline void add_counter(std::atomic<uint64_t> &counter, string_tag<chars...>) noexcept
{
    auto counter_map = get_counter_map();
    let tag = std::string{chars...};
    counter_map.insert(tag, &counter);
}

template<char... chars>
inline uint64_t get_counter(string_tag<chars...>) noexcept
{
    auto counter = get_counter_ref<chars...>();
    return counter.load(std::memory_order_relaxed);
}

inline uint64_t get_counter(std::string tag) noexcept
{
    auto counter_map = get_counter_map();
    auto counter = counter_map.get(tag);
    required_assert(counter != nullptr);
    return counter->load(std::memory_order_relaxed);
}

inline bool has_counter(std::string tag) noexcept
{
    auto counter_map = get_counter_map();
    auto i = counter_map.find(tag);
    return i != counter_map.end();
}

template<char... chars>
inline uint64_t increment_counter(string_tag<chars...>) noexcept
{
    auto &counter = get_counter_ref<chars...>();
    auto value = counter.fetch_add(1, std::memory_order_relaxed);

    if (value == 0) {
        add_counter<chars...>(counter);
    }
    return value;
}

}
