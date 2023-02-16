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
    constexpr text_theme& operator=(text_theme const&) noexcept = default;
    constexpr text_theme& operator=(text_theme&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(text_theme const&, text_theme const&) = default;

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

    void set(std::vector<text_style> const& styles) noexcept
    {
        hilet lock = std::scoped_lock(_themes_mutex);
        hi_assert(not styles.empty());
        _themes[_id] = styles;
    }

    [[nodiscard]] text_style
    operator()(text_phrasing phrasing, iso_639 language, iso_3166 region, iso_15924 script) const noexcept
    {
        hilet lock = std::scoped_lock(_themes_mutex);

        for (hilet& style : _themes[_id]) {
            if (matches(style, phrasing, language, region, script)) {
                return style;
            }
        }
        hi_axiom(not _themes[_id].empty());
        return _themes[_id].back();
    }

    /** Return the default text style.
     */
    [[nodiscard]] text_style operator*() const noexcept
    {
        hilet lock = std::scoped_lock(_themes_mutex);
        return _themes[_id].back();
    }

private:
    // 13-bit theme-id (0 through 8191).
    uint16_t _id;

    constexpr static auto _num_themes = 8192_uz;
    inline static auto _themes_mutex = unfair_mutex{};
    inline static auto _themes = std::array<std::vector<text_style>, _num_themes>{};
};

} // namespace hi::inline v1
