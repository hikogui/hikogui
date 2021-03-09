// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "axis_aligned_rectangle.hpp"
#include "translate.hpp"

namespace tt {

[[nodiscard]] axis_aligned_rectangle fit(axis_aligned_rectangle const &bounds, axis_aligned_rectangle const &rectangle) noexcept
{
    ttlet resized_rectangle = axis_aligned_rectangle{
        rectangle.left(),
        rectangle.bottom(),
        std::min(rectangle.width(), bounds.width()),
        std::min(rectangle.height(), bounds.height())};

    ttlet translate_from_p0 = max(vector2{}, get<0>(bounds) - get<0>(resized_rectangle));
    ttlet translate_from_p3 = min(vector2{}, get<3>(bounds) - get<3>(resized_rectangle));
    return translate2{translate_from_p0 + translate_from_p3} * resized_rectangle;
}

} // namespace tt
