// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include <memory>

namespace tt {
inline namespace v1 {
class gfx_surface;

class pipeline {
public:
    gfx_surface const &surface;

    pipeline(gfx_surface const &surface);

    virtual ~pipeline() = default;
    pipeline(const pipeline &) = delete;
    pipeline &operator=(const pipeline &) = delete;
    pipeline(pipeline &&) = delete;
    pipeline &operator=(pipeline &&) = delete;
};

}
}
