// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file arguments.hpp
 * This file includes functions for manipulating parameter-packs.
 */

#pragma once

namespace hi::inline v1 {

template<std::size_t I, typename FirstArg, typename... Args>
constexpr decltype(auto) get_argument_impl(FirstArg &&first_arg, Args &&...args) noexcept
{
    if constexpr (I == 0) {
        return std::forward<FirstArg>(first_arg);
    } else {
        return get_argument_impl<I - 1>(std::forward<Args>(args)...);
    }
}

template<std::size_t I, typename... Args>
constexpr decltype(auto) get_argument(Args &&...args) noexcept
{
    static_assert(I < sizeof...(Args), "Index to high for number of arguments");
    return get_argument_impl<I>(std::forward<Args>(args)...);
}

template<typename... Args>
constexpr decltype(auto) get_last_argument(Args &&...args) noexcept
{
    return get_argument_impl<sizeof...(Args) - 1>(std::forward<Args>(args)...);
}

} // namespace hi::inline v1
