// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../geometry/axis.hpp"
#include <memory>
#include <functional>

namespace hi::inline v1 {
template<axis>
class row_column_widget;

template<axis Axis>
class row_column_delegate {
public:
    virtual ~row_column_delegate() = default;
    virtual void init(row_column_widget<Axis> &sender) noexcept {}
    virtual void deinit(row_column_widget<Axis> &sender) noexcept {}
};

using row_delegate = row_column_delegate<axis::row>;
using column_delegate = row_column_delegate<axis::column>;

} // namespace hi::inline v1
