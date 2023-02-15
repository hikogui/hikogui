// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_style.hpp"
#include "../concurrency/module.hpp"
#include <vector>
#include <array>
#include <algorithm>
#include <cstdint>

namespace hi::inline v1 {



class text_theme {
public:
    constexpr text_theme() noexcept = default;
    constexpr text_theme(text_theme const&) noexcept = default;
    constexpr text_theme(text_theme&&) noexcept = default;
    constexpr text_theme &operator=(text_theme const&) noexcept = default;
    constexpr text_theme &operator=(text_theme&&) noexcept = default;

    constexpr text_theme(intrinsic_t, uint16_t id) noexcept : _id(id)
    {
        hi_axiom(_id < _num_themes);
    }

    [[nodiscard]] constexpr uint16_t const& intrinsic() const noexcept
    {
        return _id;
    }

    [[nodiscard]] constexpr uint16_t& intrinsic() noexcept
    {
        return _id;
    }

    void clear() noexcept
    {
        hilet lock = std::scoped_lock(_themes_mutex);
        _themes[_id].clear();
    }

    void set(std::vector<text_style> const &styles) noexcept
    {
        hilet lock = std::scoped_lock(_themes_mutex);
        _themes[_id] = styles; 
    }

private:

    // 13-bit theme-id (0 through 8191).
    uint16_t _id;

    constexpr static auto _num_themes = 8192_uz;
    inline static auto _themes_mutex = unfair_mutex{};
    inline static auto _themes = std::array<std::vector<text_style>, _num_themes>{};
};

} // namespace hi::inline v1
