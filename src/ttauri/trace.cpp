// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "trace.hpp"

namespace tt {

void trace_record() noexcept
{
    auto &stack = trace_stack;
    if (stack.record_depth < stack.depth) {
        stack.record_depth = stack.depth;
    }
}

}
