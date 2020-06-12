// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace tt {

constexpr inline bool format_uses_arg_ids(const char *fmt)
{
    bool start_placeholder = false;

    while (true) {
        ttlet c = *(fmt++);
        if (c == 0) {
            return false;
        } else if (c == '{') {
            start_placeholder = true;
        } else if (start_placeholder && c >= '0' && c <= '9') {
            return true;
        } else {
            start_placeholder = false;
        }
    }
}


}