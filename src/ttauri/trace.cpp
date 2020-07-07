// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/trace.hpp"

namespace tt {

void trace_record() noexcept
{
    auto &stack = trace_stack;
    if (stack.record_depth < stack.depth) {
        stack.record_depth = stack.depth;
    }
}

}
