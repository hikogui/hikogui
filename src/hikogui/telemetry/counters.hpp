// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file counters.hpp
 */

#pragma once

#include "log.hpp"
#include "../utility/utility.hpp"
#include "../concurrency/concurrency.hpp"
#include "../concurrency/unfair_mutex.hpp" // XXX #616
#include "../concurrency/thread.hpp" // XXX #616
#include "../time/time.hpp"
#include "../macros.hpp"
#include <span>
#include <typeinfo>
#include <typeindex>
#include <string>
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <chrono>
#include <limits>

hi_export_module(hikogui.telemetry : counters);


hi_export namespace hi::inline v1 {
namespace detail {

class counter {
public:
    /** Get the named counter.
     *
     * @pre main() must have been started.
     * @param name The name of the counter.
     * @return A pointer to the counter, or nullptr if the counter is not found.
     */
    [[nodiscard]] static counter *get_if(std::string const& name) noexcept
    {
        hilet lock = std::scoped_lock(_mutex);
        hilet& map_ = _map.get_or_make();
        hilet it = map_.find(name);
        if (it == map_.cend()) {
            return nullptr;
        } else {
            hi_assert(it->second);
            return it->second;
        }
    }

    counter(counter const&) = delete;
    counter(counter&&) = delete;
    counter& operator=(counter const&) = delete;
    counter& operator=(counter&&) = delete;

    constexpr counter() noexcept {}

    operator uint64_t() const noexcept
    {
        return _total_count.load(std::memory_order::relaxed);
    }

    static void log() noexcept
    {
        hilet lock = std::scoped_lock(_mutex);
        log_header();
        for (hilet & [ string, counter ] : _map.get_or_make()) {
            hi_assert(counter);
            counter->log(string);
        }
    }

    static void log_header() noexcept
    {
        hi_log_statistics("");
        hi_log_statistics("{:>18} {:>9} {:>10} {:>10} {:>10}", "total", "delta", "min", "max", "mean");
        hi_log_statistics("------------------ --------- ---------- ---------- ----------");
    }

    /** Log the counter.
     */
    /** Log the counter.
     */
    void log(std::string const& tag) noexcept
    {
        hilet total_count = _total_count.load(std::memory_order::relaxed);
        hilet prev_count = _prev_count.exchange(_total_count, std::memory_order::relaxed);
        hilet delta_count = total_count - prev_count;
        if (delta_count != 0) {
            hilet duration_max = time_stamp_count::duration_from_count(_duration_max.exchange(0, std::memory_order::relaxed));
            hilet duration_min = time_stamp_count::duration_from_count(
                _duration_min.exchange(std::numeric_limits<uint64_t>::max(), std::memory_order::relaxed));

            hilet duration_avg = _duration_avg.exchange(0, std::memory_order::relaxed);
            if (duration_avg == 0) {
                hi_log_statistics("{:>18} {:>+9} {:10} {:10} {:10} {}", total_count, delta_count, "", "", "", tag);

            } else {
                hilet avg_count = duration_avg & 0xffff;
                hilet avg_sum = duration_avg >> 16;
                hilet average = time_stamp_count::duration_from_count(avg_sum / avg_count);

                hi_log_statistics(
                    "{:18d} {:+9d} {:>10} {:>10} {:>10} {}",
                    total_count,
                    delta_count,
                    format_engineering(duration_min),
                    format_engineering(duration_max),
                    format_engineering(average),
                    tag);
            }
        }
    }

    uint64_t operator++() noexcept
    {
        return _total_count.fetch_add(1, std::memory_order::relaxed) + 1;
    }

    uint64_t operator++(int) noexcept
    {
        return _total_count.fetch_add(1, std::memory_order::relaxed);
    }

    uint64_t operator--() noexcept
    {
        return _total_count.fetch_sub(1, std::memory_order::relaxed) + 1;
    }

    uint64_t operator--(int) noexcept
    {
        return _total_count.fetch_sub(1, std::memory_order::relaxed);
    }

    /** Add a duration.
     */
    void add_duration(uint64_t duration) noexcept
    {
        _total_count.fetch_add(1, std::memory_order::relaxed);
        fetch_max(_duration_max, duration, std::memory_order::relaxed);
        fetch_min(_duration_min, duration, std::memory_order::relaxed);

        // Combine duration with count in a single atomic.
        hi_axiom(duration <= (std::numeric_limits<uint64_t>::max() >> 10));
        duration <<= 16;
        ++duration;
        _duration_avg.fetch_add(duration, std::memory_order::relaxed);
    }

protected:
    using map_type = std::map<std::string, counter *>;

    /** Mutex for managing _map.
     * We disable the dead_lock_detector, so that this mutex can be used before main().
     */
    constinit static hi_inline unfair_mutex_impl<false> _mutex;
    constinit static hi_inline atomic_unique_ptr<map_type> _map;

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

template<fixed_string Tag>
class tagged_counter : public counter {
public:
    tagged_counter() noexcept : counter()
    {
        hilet lock = std::scoped_lock(_mutex);
        _map.get_or_make()[std::string{Tag}] = this;
    }
};

} // namespace detail

template<fixed_string Tag>
hi_inline detail::tagged_counter<Tag> global_counter;

[[nodiscard]] hi_inline detail::counter *get_global_counter_if(std::string const& name)
{
    return detail::counter::get_if(name);
}

hi_inline void log::log_thread_main(std::stop_token stop_token) noexcept
{
    using namespace std::chrono_literals;

    set_thread_name("log");
    hi_log_info("log thread started");

    auto counter_statistics_deadline = std::chrono::utc_clock::now() + 1min;

    while (not stop_token.stop_requested()) {
        log_global.flush();

        hilet now = std::chrono::utc_clock::now();
        if (now >= counter_statistics_deadline) {
            counter_statistics_deadline = now + 1min;
            detail::counter::log();
        }

        std::this_thread::sleep_for(100ms);
    }

    hi_log_info("log thread finished");
}

} // namespace hi::inline v1
