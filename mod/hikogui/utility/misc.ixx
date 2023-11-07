// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file misc.ixx Utilities used by the HikoGUI library itself.
 *
 * This file includes required definitions.
 */

module;
#include "../macros.hpp"

#include <utility>
#include <cstddef>
#include <string>
#include <chrono>
#include <atomic>

export module hikogui_utility_misc;

hi_warning_push();
// C26472: Don't use static_cast for arithmetic conversions, Use brace initialization, gsl::narrow_cast or gsl::narrow (type.1).
// We do not have access to narrow_cast in this file.
hi_warning_ignore_msvc(26472);
// C26473: Don't cast between pointer types where the source type and the target type are the same (type.1).
// hi_forward need to specifically cast a value to a reference using static_cast.
hi_warning_ignore_msvc(26473);

export namespace hi {
inline namespace v1 {

/** Signed size/index into an array.
 */
using ssize_t = std::ptrdiff_t;

constexpr std::size_t operator ""_uz(unsigned long long lhs) noexcept
{
    return static_cast<std::size_t>(lhs);
}

constexpr std::size_t operator""_zu(unsigned long long lhs) noexcept
{
    return static_cast<std::size_t>(lhs);
}

constexpr std::ptrdiff_t operator""_z(unsigned long long lhs) noexcept
{
    return static_cast<std::ptrdiff_t>(lhs);
}

/** Compare then store if there was a change.
 * @return true if a store was executed.
 */
template<typename T, typename U>
[[nodiscard]] bool compare_store(T& lhs, U&& rhs) noexcept
{
    if (lhs != rhs) {
        lhs = std::forward<U>(rhs);
        return true;
    } else {
        return false;
    }
}

/** Compare then store if there was a change.
 *
 * @note This atomic version does an lhs.exchange(rhs, std::memory_order_relaxed)
 * @return true if a store was executed.
 */
template<typename T, typename U>
[[nodiscard]] bool compare_store(std::atomic<T>& lhs, U&& rhs) noexcept
{
    return lhs.exchange(rhs, std::memory_order::relaxed) != rhs;
}

/** Tag used for special functions or constructions to do a override compared to another function of the same name
 */
struct override_t {};

/** Tag used in constructors to set the intrinsic value of that object.
 *
 * Those objects are also expected to include a `intrinsic()` getter function returning
 * a reference to the intrinsic value of that object.
 */
struct intrinsic_t {};
constexpr auto intrinsic = intrinsic_t{};

/** A type that can not be constructed, copied, moved or destructed.
 */
struct unusable_t {
    unusable_t() = delete;
    ~unusable_t() = delete;
    unusable_t(unusable_t const&) = delete;
    unusable_t(unusable_t&&) = delete;
    unusable_t& operator=(unusable_t const&) = delete;
    unusable_t& operator=(unusable_t&&) = delete;
};

template<class T, class U>
[[nodiscard]] constexpr auto&& forward_like(U&& x) noexcept
{
    constexpr bool is_adding_const = std::is_const_v<std::remove_reference_t<T>>;
    if constexpr (std::is_lvalue_reference_v<T&&>) {
        if constexpr (is_adding_const) {
            return std::as_const(x);
        } else {
            return static_cast<U&>(x);
        }
    } else {
        if constexpr (is_adding_const) {
            return std::move(std::as_const(x));
        } else {
            return std::move(x);
        }
    }
}

/** Get a line from an input string, upto a maximum size.
 *
 * @post The input stream is read upto and including the line termination.
 * @param in The input stream.
 * @param max_size The maximum number of characters to read.
 * @return A string containing a line of characters, excluding the line termination.
 */
template<typename CharT, typename Traits = std::char_traits<CharT>>
[[nodiscard]] std::basic_string<CharT, Traits> getline(std::basic_istream<CharT, Traits>& in, size_t max_size) noexcept
{
    auto r = std::basic_string<CharT, Traits>{};

    while (r.size() < max_size) {
        auto c = in.get();
        if (c == Traits::eof()) {
            break;

        } else if (c == '\r') {
            c = in.get();
            if (c != '\n') {
                in.unget();
            }
            break;

        } else if (c == '\n') {
            break;
        }

        r += Traits::to_char_type(c);
    }

    return r;
}

} // namespace v1
} // namespace hi::v1

hi_warning_pop();
