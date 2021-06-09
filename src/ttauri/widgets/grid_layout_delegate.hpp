// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memory>
#include <functional>

namespace tt {
class grid_layout_widget;

class grid_layout_delegate {
public:
    virtual ~grid_layout_delegate() {}
    grid_layout_delegate() noexcept {}
    grid_layout_delegate(grid_layout_delegate const &) = delete;
    grid_layout_delegate(grid_layout_delegate &&) = delete;
    grid_layout_delegate &operator=(grid_layout_delegate const &) = delete;
    grid_layout_delegate &operator=(grid_layout_delegate &&) = delete;

    virtual void init(grid_layout_widget &sender) noexcept {}

    virtual void deinit(grid_layout_widget &sender) noexcept {}
};

} // namespace tt
