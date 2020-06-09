// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/assert.hpp"
#include "TTauri/Foundation/TT5.hpp"
#include <string>
#include <string_view>
#include <exception>
#include <cstdint>
#include <typeinfo>
#include <typeindex>

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
constexpr string_ltag operator""_ltag(char const *str, size_t) noexcept
{
    return tt5_encode<string_ltag>(str);
}


template<typename Head, typename... Tail>
size_t count_tag_if_impl(std::type_index tag, size_t count) {
    if constexpr (sizeof...(Tail) > 0) {
        return count_tag_if_impl<Tail...>(tag, tag == std::type_index(typeid(Head)) ? count + 1 : count);
    } else {
        return tag == std::type_index(typeid(Head)) ? count + 1 : count;
    }
}

/*! Return the how many times tag is in the template arguments.
*/
template<typename... Tags>
size_t count_tag_if(std::type_index tag) {
    if constexpr (sizeof...(Tags) == 0) {
        return 0;
    } else {
        return count_tag_if_impl<Tags...>(tag, 0);
    }
}

template<typename Needle, typename Head, typename... Tail>
constexpr size_t count_tag_if_impl(size_t count) {
    if constexpr (sizeof...(Tail) > 0) {
        return count_tag_if_impl<Tail...>(std::is_same_v<Needle,Head> ? count + 1 : count);
    } else {
        return std::is_same_v<Needle,Head> ? count + 1 : count;
    }
}

/*! Return the how many times tag is in the template arguments.
*/
template<typename Needle, typename... Tags>
constexpr size_t count_tag_if() {
    if constexpr (sizeof...(Tags) == 0) {
        return 0;
    } else {
        return count_tag_if_impl<Needle,Tags...>(0);
    }
}

template<typename Head, typename... Tail>
std::type_index tag_at_index_impl(size_t index) noexcept
{
    if constexpr (sizeof...(Tail) > 0) {
        return index == 0 ? std::type_index(typeid(Head)) : tag_at_index_impl<Tail...>(index - 1);
    } else {
        return index == 0 ? std::type_index(typeid(Head)) : std::type_index(typeid(void));
    }
}

/*! Return the tag in the template arguments at the index.
* If the index points beyond the template arguments it will return the 0-tag.
*/
template<typename... Tags>
std::type_index tag_at_index(size_t index) noexcept
{
    if constexpr (sizeof...(Tags) > 0) {
        return tag_at_index_impl<Tags...>(index);
    } else {
        return std::type_index(typeid(void));
    }
}

template<typename Head, typename... Tail>
size_t index_of_tag_impl(std::type_index tag, size_t index) noexcept
{
    if constexpr (sizeof...(Tail) > 0) {
        return tag == std::type_index(typeid(Head)) ? index : index_of_tag_impl<Tail...>(tag, index + 1);
    } else {
        return tag == std::type_index(typeid(Head)) ? index : index + 1;
    }
}

/*! Return the index of the tag in the template arguments.
 * If the tag is not found it returns the index 1 beyond the template arguments.
 */
template<typename... Tags>
size_t index_of_tag(std::type_index tag) noexcept
{
    if constexpr (sizeof...(Tags) > 0) {
        return index_of_tag_impl<Tags...>(tag, 0);
    } else {
        return 1;
    }
}

template<typename Needle, typename Head, typename... Tail>
constexpr size_t index_of_tag_impl(size_t index) noexcept
{
    if constexpr (sizeof...(Tail) > 0) {
        return std::is_same_v<Needle,Head> ? index : index_of_tag_impl<Needle,Tail...>(index + 1);
    } else {
        return std::is_same_v<Needle,Head> ? index : index + 1;
    }
}

/*! Return the index of the tag in the template arguments.
* If the tag is not found it returns the index 1 beyond the template arguments.
*/
template<typename Needle, typename... Haystack>
constexpr size_t index_of_tag() noexcept
{
    if constexpr (sizeof...(Haystack) > 0) {
        return index_of_tag_impl<Needle, Haystack...>(0);
    } else {
        return 1;
    }
}

template<typename Needle, typename... Haystack>
constexpr bool has_tag() noexcept {
    return index_of_tag<Needle, Haystack...>() < sizeof...(Haystack);
}

template<typename... Haystack>
bool has_tag(std::type_index needle) noexcept {
    return index_of_tag<Haystack...>(needle) < sizeof...(Haystack);
}


}
