// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <ranges>
#include <algorithm>
#include <concepts>
#include <type_traits>
#include <vector>
#include <exception>
#include <stdexcept>

export module hikogui_algorithm_ranges;
import hikogui_utility;

export namespace hi::inline v1 {

template<typename Value, typename Range>
[[nodiscard]] constexpr Value get_first(Range &&range)
{
    auto it = std::ranges::begin(range);
    auto last = std::ranges::end(range);

    if (it == last) {
        throw std::out_of_range{"Range is empty"};
    }

    auto value = *it++;
    return Value{value};
}

/** @see make_vector
 */
template<typename Range>
[[nodiscard]] constexpr Range::value_type get_first(Range&& range)
{
    return get_first<typename Range::value_type>(std::forward<Range>(range));
}

/** Make a vector from a view.
 * This function will make a vector with a copy of the elements of a view.
 */
template<typename Value, typename Range>
[[nodiscard]] constexpr std::vector<Value> make_vector(Range&& range)
{
    hilet first = std::ranges::begin(range);
    hilet last = std::ranges::end(range);

    if constexpr (requires(std::vector<Value> & x) { std::ranges::copy(first, last, std::back_inserter(x)); }) {
        // This should handle almost everything.
        auto r = std::vector<Value>{};
        if constexpr (requires { std::distance(first, last); }) {
            r.reserve(std::distance(first, last));
        }
        std::ranges::copy(first, last, std::back_inserter(r));
        return r;

    } else if constexpr (requires { Value{std::string_view{(*first).begin(), (*first).end()}}; }) {
        // std::views::split returns a range of ranges, handle the string_view cases.
        auto r = std::vector<Value>{};
        if constexpr (requires { std::distance(first, last); }) {
            r.reserve(std::distance(first, last));
        }
        for (auto it = first; it != last; ++it) {
            r.emplace_back(std::string_view{(*it).begin(), (*it).end()});
        }
        return r;

    } else {
        hi_static_not_implemented();
    }
}

/** @see make_vector
 */
template<typename Range>
[[nodiscard]] constexpr std::vector<typename Range::value_type> make_vector(Range&& range)
{
    return make_vector<typename Range::value_type>(std::forward<Range>(range));
}

} // namespace hi::inline v1
