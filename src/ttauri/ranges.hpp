// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "coroutine.hpp"
#include "type_traits.hpp"
#include <ranges>
#include <algorithm>
#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace tt::inline v1 {

/** Make a vector from a view.
 * This function will make a vector with a copy of the elements of a view.
 */
template<typename View>
[[nodiscard]] std::vector<typename View::value_type> make_vector(View const &view)
{
    auto r = std::vector<View::value_type>{};
    auto first = begin(view);
    auto last = end(view);
    r.reserve(std::distance(first, last));
    std::copy(first, last, std::back_inserter(r));
    return r;
}

/** Make a vector from a view.
 * This function will make a vector with a by moving the elements of a view.
 */
template<std::ranges::sized_range View>
[[nodiscard]] std::vector<typename View::value_type> make_vector(View &&view) noexcept
{
    auto r = std::vector<View::value_type>{};
    auto first = begin(view);
    auto last = end(view);
    r.reserve(std::distance(first, last));
    std::ranges::copy(first, last, std::back_inserter(r));
    return r;
}

/** Make a vector from a view.
 * This function will make a vector with a by moving the elements of a view.
 */
template<typename View>
[[nodiscard]] std::vector<typename View::value_type> make_vector(View &&view) noexcept
{
    auto r = std::vector<View::value_type>{};
    auto first = begin(view);
    auto last = end(view);
    std::ranges::copy(first, last, std::back_inserter(r));
    return r;
}

} // namespace tt::inline v1