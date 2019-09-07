// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <type_traits>

namespace TTauri {

template<typename T, typename Head, typename... Tail>
constexpr size_t count_if(size_t count = 0) {
    if constexpr (sizeof...(Tail) > 0) {
        return count_if<T, Tail...>(std::is_same_v<T, Head> ? count + 1 : count);
    } else {
        return std::is_same_v<T, Head> ? count + 1 : count;
    }
}

template<typename T, typename Head, typename... Tail>
constexpr size_t index_of(size_t index = 0) {
    static_assert(std::is_same_v<T, Head> || sizeof...(Tail) > 0, "Could not find type");
    if constexpr (sizeof...(Tail) > 0) {
        return (std::is_same_v<T, Head>) ? index : index_of<T, Tail...>(index + 1);
    } else {
        return index;
    }
}

}