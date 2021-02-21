// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <type_traits>
#include <string>
#include <string_view>

#pragma once

namespace tt {

/** Functor for forwarding an forwarding-reference to variable.
 *
 * This functor is used to long-time-storage of values passed into a template function.
 * This means that views need to be translated into non-view values.
 *
 * For savety against the lifetime of the orginal object ending:
 *  - rvalues are moved
 *  - lvalues are copied
 *  - std::string_view are copied into a std::string
 *  - std::span are copied into a std::vector
 *
 * For performance a string literal which is stored in constant are taken by
 * pointer:
 *  - char const (&)[] (string literal) are forward as a char const *
 */
template<typename T>
struct forward_value {
    using type = std::remove_cvref_t<T>;

    [[nodiscard]] constexpr T &&operator()(std::remove_reference_t<T> &t) const noexcept
    {
        return static_cast<T &&>(t);
    }

    [[nodiscard]] constexpr T &&operator()(std::remove_reference_t<T> &&t) const noexcept
    {
        static_assert(!std::is_lvalue_reference_v<T>, "Can not forward an rvalue as an lvalue.");
        return static_cast<T &&>(t);
    }
};

template<size_t N>
struct forward_value<char const (&)[N]> {
    using type = char const *;

    [[nodiscard]] constexpr char const *operator()(char const (&t)[N]) const noexcept
    {
        return static_cast<char const *>(t);
    }
};

template<>
struct forward_value<std::string_view> {
    using type = std::string;

    [[nodiscard]] std::string operator()(std::string_view t) const noexcept
    {
        return std::string{t};
    }
};

// Override for char const *, otherwise forward_value will match
// char const (&)[].
template<>
struct forward_value<char const *> {
    using type = std::string;

    [[nodiscard]] std::string operator()(char const *t) const noexcept
    {
        return std::string{t};
    }
};


/** Get the storage type of the `forward_value` functor.
 * Use this type for the variables that are assigned with the return
 * value of the `forward_value` functor.
 */
template<typename T>
using forward_value_t = typename forward_value<T>::type;

} // namespace tt
