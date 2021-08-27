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
class counter;

/** A list of counters.
 */
inline std::map<std::string, counter *> counter_map = {};

template<basic_fixed_string Tag>
class counter {
public:
    counter(counter const &) = delete;
    counter(counter &&) = delete;
    counter &operator=(counter const &) = delete;
    counter &operator=(counter &&) = delete;

    counter() noexcept
    {
        counter_map[static_cast<std::string>(Tag)] = this;
    }

    operator uint64_t () const noexcept
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

private:
    std::atomic<uint64_t> _v = 0;
    uint64_t _prev_v = 0;
};

}

template<basic_fixed_string Tag>
inline detail::counter<Tag> counter;

} // namespace tt
