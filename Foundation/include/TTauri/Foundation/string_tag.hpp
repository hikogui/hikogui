// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/assert.hpp"
#include "TTauri/Foundation/TT5.hpp"
#include <string>
#include <string_view>
#include <exception>
#include <cstdint>

namespace TTauri {

using string_tag = tt5_64;
using string_ltag = tt5_128;

/*! _tag literal
*/
constexpr string_tag operator""_tag(char const *str, size_t) noexcept
{
    return tt5_encode<string_tag>(str);
}

/*! _tag literal
*/
constexpr string_tag operator""_ltag(char const *str, size_t) noexcept
{
    return tt5_encode<string_ltag>(str);
}


template<string_tag Head, string_tag... Tail>
constexpr size_t count_tag_if_impl(string_tag tag, size_t count) {
    if constexpr (sizeof...(Tail) > 0) {
        return count_tag_if_impl<Tail...>(tag, tag == Head ? count + 1 : count);
    } else {
        return tag == Head ? count + 1 : count;
    }
}

/*! Return the how many times tag is in the template arguments.
*/
template<string_tag... Tags>
constexpr size_t count_tag_if(string_tag tag) {
    if constexpr (sizeof...(Tags) == 0) {
        return 0;
    } else {
        return count_tag_if_impl<Tags...>(tag, 0);
    }
}

template<string_tag Head, string_tag... Tail>
constexpr string_tag tag_at_index_impl(size_t index) noexcept
{
    if constexpr (sizeof...(Tail) > 0) {
        return index == 0 ? Head : tag_at_index_impl<Tail...>(index - 1);
    } else {
        return index == 0 ? Head : 0;
    }
}

/*! Return the tag in the template arguments at the index.
* If the index points beyond the template arguments it will return the 0-tag.
*/
template<string_tag... Tags>
constexpr string_tag tag_at_index(size_t index) noexcept
{
    if constexpr (sizeof...(Tags) > 0) {
        return tag_at_index_impl<Tags...>(index);
    } else {
        return 0;
    }
}

template<string_tag Head, string_tag... Tail>
constexpr size_t index_of_tag_impl(string_tag tag, size_t index) noexcept
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
template<string_tag... Tags>
constexpr size_t index_of_tag(string_tag tag) noexcept
{
    if constexpr (sizeof...(Tags) > 0) {
        return index_of_tag_impl<Tags...>(tag, 0);
    } else {
        return 1;
    }
}

}
