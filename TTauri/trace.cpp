// Copyright 2019 Pokitec
// All rights reserved.

#include "trace.hpp"
#include "Application.hpp"
#include <fmt/format.h>

namespace TTauri {

void trace_record() noexcept
{
    auto &stack = trace_stack;
    if (stack.record_depth < stack.depth) {
        stack.record_depth = stack.depth;
    }
}

std::ostream &operator<<(std::ostream &lhs, trace_data const &rhs)
{
    auto info_string = std::string{};

    auto counter = 0;
    for (let &item: rhs.trace_info) {
        if (counter++ > 0) {
            info_string += ", ";
            info_string += tag_to_string(item.key);
            info_string += "=";
            info_string += static_cast<std::string>(item.value);
        }
    }

    lhs << fmt::format("tag={} id={} parent={} start={} {}",
        tag_to_string(rhs.tag), rhs.id, rhs.parent_id,
        format_full_datetime(hiperf_utc_clock::convert(rhs.timestamp), application().time_zone),
        info_string
    );
    return lhs;
}

}