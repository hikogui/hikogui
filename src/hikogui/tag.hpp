// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "strings.hpp"
#include "fixed_string.hpp"
#include <string>
#include <string_view>
#include <exception>
#include <cstdint>
#include <typeinfo>
#include <typeindex>

namespace hi::inline v1 {

template<fixed_string Head, fixed_string... Tail>
std::string tag_at_index_impl(std::size_t index) noexcept
{
    if constexpr (sizeof...(Tail) > 0) {
        return index == 0 ? Head : tag_at_index_impl<Tail...>(index - 1);
    } else {
        return index == 0 ? Head : std::string{};
    }
}

/*! Return the tag in the template arguments at the index.
 * If the index points beyond the template arguments it will return the 0-tag.
 */
template<fixed_string... Tags>
std::string tag_at_index(std::size_t index) noexcept
{
    if constexpr (sizeof...(Tags) > 0) {
        return tag_at_index_impl<Tags...>(index);
    } else {
        return {};
    }
}

template<fixed_string Head, fixed_string... Tail>
std::size_t index_of_tag_impl(std::string tag, std::size_t index) noexcept
{
    if constexpr (sizeof...(Tail) > 0) {
        return tag == Head ? index : index_of_tag_impl<Tail...>(tag, index + 1);
    } else {
        return tag == Head ? index : index + 1;
    }
}

/*! Return the index of the tag in the template arguments.
 * If the tag is not found it returns the index 1 beyond the template arguments.
 */
template<fixed_string... Tags>
std::size_t index_of_tag(std::string tag) noexcept
{
    if constexpr (sizeof...(Tags) > 0) {
        return index_of_tag_impl<Tags...>(tag, 0);
    } else {
        return 1;
    }
}

template<fixed_string Needle, fixed_string Head, fixed_string... Tail>
constexpr std::size_t index_of_tag_impl(std::size_t index) noexcept
{
    if constexpr (sizeof...(Tail) > 0) {
        return Needle == Head ? index : index_of_tag_impl<Needle, Tail...>(index + 1);
    } else {
        return Needle == Head ? index : index + 1;
    }
}

/*! Return the index of the tag in the template arguments.
 * If the tag is not found it returns the index 1 beyond the template arguments.
 */
template<fixed_string Needle, fixed_string... Haystack>
constexpr std::size_t index_of_tag() noexcept
{
    if constexpr (sizeof...(Haystack) > 0) {
        return index_of_tag_impl<Needle, Haystack...>(0);
    } else {
        return 1;
    }
}

template<fixed_string Needle, fixed_string... Haystack>
constexpr bool has_tag() noexcept
{
    return index_of_tag<Needle, Haystack...>() < sizeof...(Haystack);
}

template<fixed_string... Haystack>
bool has_tag(std::string needle) noexcept
{
    return index_of_tag<Haystack...>(needle) < sizeof...(Haystack);
}

} // namespace hi::inline v1
