// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <type_traits>
#include <string>
#include <string_view>

#pragma once

namespace hi::inline v1 {

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

    [[nodiscard]] type operator()(T const &t) const noexcept
    {
        return t;
    }
};

#define MAKE_FORWARD_VALUE(TEMPLATE_TYPE, RETURN_TYPE, ARGUMENT_TYPE) \
    template<> \
    struct forward_value<TEMPLATE_TYPE> { \
        using type = RETURN_TYPE; \
\
        [[nodiscard]] type operator()(ARGUMENT_TYPE t) const noexcept \
        { \
            return type{t}; \
        } \
    };

// Copy string_view by string value.
MAKE_FORWARD_VALUE(std::string_view, std::string, std::string_view const &)
MAKE_FORWARD_VALUE(std::string_view const, std::string, std::string_view const &)
MAKE_FORWARD_VALUE(std::string_view &, std::string, std::string_view const &)
MAKE_FORWARD_VALUE(std::string_view const &, std::string, std::string_view const &)

// Copy char pointers by string value.
MAKE_FORWARD_VALUE(char *, std::string, char const *)
MAKE_FORWARD_VALUE(char const *, std::string, char const *)
MAKE_FORWARD_VALUE(char *const, std::string, char const *)
MAKE_FORWARD_VALUE(char const *const, std::string, char const *)
MAKE_FORWARD_VALUE(char *&, std::string, char const *)
MAKE_FORWARD_VALUE(char const *&, std::string, char const *)
MAKE_FORWARD_VALUE(char *const &, std::string, char const *)
MAKE_FORWARD_VALUE(char const *const &, std::string, char const *)

#undef MAKE_FORWARD_VALUE

// Copy string literal by pointer.
template<std::size_t N>
struct forward_value<char const (&)[N]> {
    using type = char const *;

    [[nodiscard]] constexpr type operator()(char const (&t)[N]) const noexcept
    {
        return static_cast<char const *>(t);
    }
};

/** Get the storage type of the `forward_value` functor.
 * Use this type for the variables that are assigned with the return
 * value of the `forward_value` functor.
 */
template<typename T>
using forward_value_t = typename forward_value<T>::type;

} // namespace hi::inline v1
