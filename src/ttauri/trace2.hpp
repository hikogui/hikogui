// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "fixed_string.hpp"

namespace tt {
namespace detail {

class trace_stats {
public:
    std::map<std::string,trace_stats *> map;

    trace_stats(trace_stats const &) = delete;
    trace_stats(trace_stats &&) = delete;
    trace_stats &operator=(trace_stats const &) = delete;
    trace_stats &operator=(trace_stats &&) = delete;

    trace_stats() noexcept {}

};

template<basic_fixed_string Tag>
class tagged_trace_stats : public trace_stats {
public:
    tagged_trace_stats() noexcept : trace_stats() {
        map[Tag] = this;
    }
};

}

template<basic_fixed_string Tag>
inline detail::tagged_trace_stats<Tag> trace_stats;


template<fixed_string Tag>
class trace {
public:
    trace(trace const &) = delete;
    trace(trace &&) = delete;
    trace &operator=(trace const &) = delete;
    trace &operator=(trace &&) = delete;

    ~trace() noexcept
    {
        if (std::uncaught_exceptions()) {
        }
        top = _next;
    }

    trace() noexcept : _next(std::exchange(top, this));
    {
    }

private:
    static thread_local trace *top = nullptr;
    trace *_next = nullptr;
};


};

