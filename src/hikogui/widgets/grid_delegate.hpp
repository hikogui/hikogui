// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>
#include <functional>

namespace hi::inline v1 {
class grid_widget;

class grid_delegate {
public:
    virtual ~grid_delegate() = default;

    virtual void init(grid_widget &sender) noexcept {}

    virtual void deinit(grid_widget &sender) noexcept {}
};

} // namespace hi::inline v1
