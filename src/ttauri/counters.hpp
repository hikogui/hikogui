// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file counter.hpp
 */

#pragma once

#include "architecture.hpp"
#include "fixed_string.hpp"
#include "statistics.hpp"
#include <span>
#include <typeinfo>
#include <typeindex>
#include <string>
#include <map>

namespace tt {
namespace detail {

class counter {
public:
    static inline std::map<std::string, counter *> map = {};

    counter(counter const &) = delete;
    counter(counter &&) = delete;
    counter &operator=(counter const &) = delete;
    counter &operator=(counter &&) = delete;

    constexpr counter() noexcept {}

    operator uint64_t() const noexcept
    {
        return _v.load(std::memory_order::relaxed);
    }

    /** Read current and previous value of the counter.
     *
     * @pre Should be called from the statistics thread.
     * @return current, previous counter value.
     */
    [[nodiscard]] std::pair<uint64_t, uint64_t> read() const noexcept
    {
        ttlet current = _v.load(std::memory_order::relaxed);
        ttlet previous = std::exchange(_prev_v, current);
        return {current, previous};
    }

    counter &operator++() noexcept
    {
        _v.fetch_add(1, std::memory_order::relaxed);
        return *this;
    }

    uint64_t operator++(int) noexcept
    {
        return _v.fetch_add(1, std::memory_order::relaxed);
    }

    /** Add a duration.
     */
    void add_duration(uint64_t duration) noexcept
    {
        ttlet old_version = _count.fetch_add(1, std::memory_order::acquire);

        _duration_sum.fetch_add(duration, std::memory_order::relaxed);

        auto expected = _duration_peak.load(std::memory_order::relaxed);
        while (expected < duration) {
            if (_duration_peak.compare_exchange_strong(expected, duration, std::memory_order::relaxed)) {
                break;
            }
        }

        _version.store(old_version + 1, std::memory_order::release);
    }

protected:
    std::atomic<uint64_t> _count = 0;
    std::atomic<uint64_t> _duration_peak = 0;    
    std::atomic<uint64_t> _duration_sum = 0;    
    std::atomic<uint64_t> _version = 0;
    uint64_t _prev_count = 0;
};

template<basic_fixed_string Tag>
class tagged_counter : public counter {
public:
    tagged_counter() noexcept : counter()
    {
        map[Tag] = this;
    }
};

} // namespace detail

template<basic_fixed_string Tag>
inline detail::tagged_counter<Tag> global_counter;

[[nodiscard]] inline detail::counter *get_global_counter(std::string const &name)
{
    ttlet it = detail::counter::map.find(name);
    if (it == detail::counter::map.cend()) {
        return nullptr;
    } else {
        tt_axiom(it->second);
        return it->second;
    }
}

} // namespace tt
