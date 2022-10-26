// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "fixed_string.hpp"
#include "time_stamp_count.hpp"
#include "counters.hpp"
#include "datum.hpp"
#include <array>
#include <tuple>

namespace hi::inline v1 {

class trace_base {
public:
    trace_base(trace_base const &) = delete;
    trace_base(trace_base &&) = delete;
    trace_base &operator=(trace_base const &) = delete;
    trace_base &operator=(trace_base &&) = delete;

    trace_base() noexcept : _time_stamp(time_stamp_count::inplace{}), _next(std::exchange(_top, this)) {}

    virtual ~trace_base()
    {
        _top = _next;
    }

    virtual void log() const noexcept = 0;

protected:
    inline static thread_local trace_base *_top = nullptr;

    time_stamp_count _time_stamp;
    trace_base *_next = nullptr;
};

template<fixed_string Tag, int NumItems = 0>
class trace : public trace_base {
public:
    trace() noexcept : trace_base(), items(), size(0) {}

    virtual ~trace() noexcept
    {
        if (std::uncaught_exceptions()) {
            log();
        }

        hilet current_time_stamp = time_stamp_count{time_stamp_count::inplace{}};
        global_counter<Tag>.add_duration(current_time_stamp.count() - _time_stamp.count());
    }

    void log() const noexcept override
    {
        if (_next) {
            _next->log();
        }
    }

    template<typename T>
    void set(char const *key, T &&value) noexcept
    {
        // XXX Use type erase to store the type of the value and a pointer
        // to the location of the value on the stack.

        hi_axiom(size < NumItems);
        items[size++] = {key, datum{std::forward<T>(value)}};
    }

private:
    std::array<std::pair<char const *, datum>, NumItems> items;
    std::size_t size = 0;
};

template<fixed_string Tag>
class trace<Tag, 0> : public trace_base {
public:
    trace() noexcept : trace_base() {}

    virtual ~trace() noexcept
    {
        if (std::uncaught_exceptions()) {
            log();
        }

        hilet current_time_stamp = time_stamp_count{time_stamp_count::inplace{}};
        global_counter<Tag>.add_duration(current_time_stamp.count() - _time_stamp.count());
    }

    void log() const noexcept override
    {
        if (_next) {
            _next->log();
        }
    }
};

} // namespace hi::inline v1
