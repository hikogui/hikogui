// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>

namespace hi::inline v1 {

// template<typename T, typename Head, typename... Tail>
// constexpr std::size_t count_type_if(std::size_t count = 0) {
//    if constexpr (sizeof...(Tail) > 0) {
//        return count_type_if<T, Tail...>(std::is_same_v<T, Head> ? count + 1 : count);
//    } else {
//        return std::is_same_v<T, Head> ? count + 1 : count;
//    }
//}
//
// template<typename T, typename Head, typename... Tail>
// constexpr std::size_t index_of_type(std::size_t index = 0) {
//    static_assert(std::is_same_v<T, Head> || sizeof...(Tail) > 0, "Could not find type");
//    if constexpr (sizeof...(Tail) > 0) {
//        return (std::is_same_v<T, Head>) ? index : index_of_type<T, Tail...>(index + 1);
//    } else {
//        return index;
//    }
//}

}