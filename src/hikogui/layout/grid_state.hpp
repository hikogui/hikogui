

#pragma once

namespace hi { inline namespace v1 {

enum class grid_state : uint8_t {
    none = 0,
    need_layout = 1,
    need_constrain = 2
};

[[nodiscard]] constexpr grid_state operator&(grid_state const& lhs, grid_state const& rhs) noexcept
{
    return static_cast<grid_state>(to_underlying(lhs) & to_underlying(rhs));
}

[[nodiscard]] constexpr grid_state operator|(grid_state const& lhs, grid_state const& rhs) noexcept
{
    return static_cast<grid_state>(to_underlying(lhs) | to_underlying(rhs));
}

constexpr grid_state& operator&(grid_state& lhs, grid_state const& rhs) noexcept
{
    return lhs = lhs & rhs;
}

constexpr grid_state& operator|(grid_state& lhs, grid_state const& rhs) noexcept
{
    return lhs = lhs | rhs;
}

[[nodiscard]] constexpr bool to_bool(grid_state const& rhs) noexcept
{
    return static_cast<bool>(to_underlying(rhs));
}

}}
