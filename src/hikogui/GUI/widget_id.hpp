// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../concurrency/concurrency.hpp"
#include "../macros.hpp"
#include <cstdint>
#include <utility>
#include <concepts>
#include <compare>

hi_export_module(hikogui.GUI : widget_id);

hi_export namespace hi { inline namespace v1 {
namespace detail {
hi_inline id_factory<uint32_t> widget_id_factory;
}

enum class widget_id : uint32_t {};

[[nodiscard]] hi_inline widget_id make_widget_id() noexcept
{
    return static_cast<widget_id>(detail::widget_id_factory.acquire());
}

hi_inline void release_widget_id(widget_id id) noexcept
{
    detail::widget_id_factory.release(std::to_underlying(id));
}

[[nodiscard]] constexpr bool operator==(widget_id const& lhs, std::integral auto const& rhs) noexcept
{
    return std::cmp_equal(std::to_underlying(lhs), rhs);
}

[[nodiscard]] constexpr std::strong_ordering operator<=>(widget_id const& lhs, std::integral auto const& rhs) noexcept
{
    if (std::cmp_equal(std::to_underlying(lhs), rhs)) {
        return std::strong_ordering::equal;
    } else if (std::cmp_less(std::to_underlying(lhs), rhs)) {
        return std::strong_ordering::less;
    } else {
        return std::strong_ordering::greater;
    }
}

}} // namespace hi::v1
