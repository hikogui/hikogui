// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "exception.hpp"
#include "utils.hpp"

namespace tt::inline v1 {

#define tt_parse_check(expression, message, ...) \
    do { \
        if (!(expression)) { \
            throw ::tt::parse_error(message __VA_OPT__(, ) __VA_ARGS__); \
        } \
    } while (false)

#define tt_hresult_check(expression) \
    ([](HRESULT result) { \
        if (FAILED(result)) { \
            throw ::tt::io_error("Call to '{}' failed with {:08x}", #expression, result); \
        } \
        return result; \
    }(expression))


/** Cast integrals to different types and signedness
 */
template<std::integral Out, std::integral In>
[[nodiscard]] constexpr Out check_cast(In in) noexcept(type_in_range_v<Out,In>)
{
    if constexpr not (type_in_range_v<Out,In>) {
        if (not std::in_range<Out>(in)) {
            throw std::out_of_range("integer cast failed");
        }
    }
    return static_cast<Out>(in);
}

/** Cast integrals to floating point.
 */
template<std::floating_point Out, std::integral In>
[[nodiscard]] constexpr Out check_cast(In in) noexcept
{
    return static_cast<Out>(in);
}

} // namespace tt::inline v1
