// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "strings.hpp"
#include <string>
#include <string_view>
#include <exception>
#include <cstdint>
#include <typeinfo>
#include <typeindex>

namespace tt {

inline std::string tag_name(std::type_index tag) noexcept
{
    ttlet long_name = std::string{tag.name()};
    ttlet split_name = split(long_name, "::"s);
    if (std::ssize(split_name) != 0) {
        ttlet &last_name = split_name.back();
        if (last_name.ends_with("_tag"s)) {
            return last_name.substr(0, std::ssize(last_name) - 4);
        } else {
            return last_name;
        }
    } else {
        return long_name;
    }
}

template<typename Tag>
std::string tag_name()
{
    return tag_name(std::type_index(typeid(Tag)));
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
