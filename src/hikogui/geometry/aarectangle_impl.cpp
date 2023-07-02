// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "aarectangle.hpp"
#include "translate2.hpp"

namespace hi::inline v1 {

[[nodiscard]] aarectangle fit(aarectangle const& bounds, aarectangle const& rectangle) noexcept
{
    hilet resized_rectangle = aarectangle{
        rectangle.left(),
        rectangle.bottom(),
        std::min(rectangle.width(), bounds.width()),
        std::min(rectangle.height(), bounds.height())};

    hilet translate_from_p0 = max(vector2{}, get<0>(bounds) - get<0>(resized_rectangle));
    hilet translate_from_p3 = min(vector2{}, get<3>(bounds) - get<3>(resized_rectangle));
    return translate2{translate_from_p0 + translate_from_p3} * resized_rectangle;
}

} // namespace hi::inline v1
