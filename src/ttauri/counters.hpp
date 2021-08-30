// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file counter.hpp
 */

#pragma once

#include "architecture.hpp"
#include "fixed_string.hpp"
#include "statistics.hpp"
#include "time_stamp_count.hpp"
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

    static void log() noexcept
    {
        log_header();
        for (ttlet &[string, counter]: map) {
            tt_axiom(counter);
            counter->log(string);
        }
    }

    static void log_header() noexcept
    {
        tt_log_statistics("{:>18} {:>9} {:>10} {:>10} {:>10}", "total", "delta", "mean", "min", "max");
    }

    /** Log the counter.
     */
    void log(std::string const &tag) const noexcept
    {
        ttlet total_count = _total_count.load(std::memory_order::relaxed);
        ttlet prev_count = _prev_count.exchange(_total_count, std::memory_order::relaxed);
        ttlet duration_max = time_stamp_count::duration_from_count(
            _duration_max.exchange(0, std::memory_order::relaxed));
        ttlet duration_min = time_stamp_count::duration_from_count(
            _duration_max.exchange(std::numeric_limits<uint64_t>::max(), std::memory_order::relaxed));

        ttlet duration_avg = _duration_avg.exchange(0, std::memory_order::relaxed);
        if (duration_avg == 0) {
            tt_log_statistics("{:>18} {:>+9} {:10} {:10} {:10} {}", total_count, total_count - prev_count, "", "", "", tag);

        } else {
            ttlet avg_count = duration_avg & 0x3'ff;
            ttlet avg_sum = duration_avg >> 10;
            ttlet average = time_stamp_count::duration_from_count(avg_sum / avg_count);

            tt_log_statistics(
                "{:18d} {:+9d} {:>10} {:>10} {:>10} {}",
                total_count,
                total_count - prev_count,
                average,
                duration_min,
                duration_max,
                tag);
        }
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
        _count.fetch_add(1, std::memory_order::relaxed);
        fetch_max(_duration_max, duration, std::memory_order::relaxed);
        fetch_min(_duration_min, duration, std::memory_order::relaxed);

        // Combine duration with count in a single atomic.
        tt_axiom(duration <= (std::numeric_limits<uint64_t>::max() >> 10));
        duration <<= 10;
        ++duration;
        _duration_avg.fetch_add(duration, std::memory_order::relaxed);
    }

protected:
    std::atomic<uint64_t> _total_count = 0;
    std::atomic<uint64_t> _prev_count = 0;
    std::atomic<uint64_t> _duration_max = 0;
    std::atomic<uint64_t> _duration_min = std::numeric_limits<uint64_t>::max();

    /** Average duration.
     *
     * - [9:0] Count.
     * - [63:10] Sum.
     */
    std::atomic<uint64_t> _duration_avg = 0;    
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
