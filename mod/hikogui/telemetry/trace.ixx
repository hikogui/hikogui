// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <array>
#include <tuple>
#include <exception>

export module hikogui_telemetry : trace;
import : counters;
import hikogui_time;
import hikogui_utility;

export namespace hi::inline v1 {

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

template<fixed_string Tag>
class trace : public trace_base {
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
