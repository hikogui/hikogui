// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file counters.hpp
 */

#pragma once

#include "utility/module.hpp"
#include "concurrency/module.hpp"
#include "time_stamp_count.hpp"
#include "atomic.hpp"
#include <span>
#include <typeinfo>
#include <typeindex>
#include <string>
#include <atomic>
#include <map>
#include <memory>
#include <mutex>

namespace hi::inline v1 {
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

    static void log_header() noexcept;

    /** Log the counter.
     */
    void log(std::string const& tag) noexcept;

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
    constinit static inline unfair_mutex_impl<false> _mutex;
    constinit static inline atomic_unique_ptr<map_type> _map;

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
inline detail::tagged_counter<Tag> global_counter;

[[nodiscard]] inline detail::counter *get_global_counter_if(std::string const& name)
{
    return detail::counter::get_if(name);
}

} // namespace hi::inline v1
