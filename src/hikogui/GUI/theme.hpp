// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "theme_mode.hpp"
#include "../text/text_style.hpp"
#include "../utility/module.hpp"
#include "../datum.hpp"
#include "../color/module.hpp"
#include "../geometry/module.hpp"
#include "../pattern_match.hpp"
#include "../generator.hpp"
#include "../concurrency/module.hpp"
#include <array>
#include <filesystem>
#include <string>
#include <vector>
#include <atomic>
#include <map>

namespace hi { inline namespace v1 {
class font_book;

/** A theme loaded from a theme file.
 *
 * The theme object is not directly used when drawing the user interface.
 * The draw function will use the theme-value API `hi::tv` to select specific values.
 *
 * When a theme is activated it will update the global `hi::tv` values.
 *
 * Loading of themes is done through the theme-book.
 */
class theme {
public:
    using value_type = std::variant<float, std::vector<hi::color>, std::vector<hi::text_style>, std::string>;
    using container_type = std::vector<std::pair<std::string, value_type>>;

    /** The name of the theme.
     *
     * The name may be repeated, once for each mode.
     */
    std::string name;

    /** The mode that this theme is used for.
     *
     * If there are multiple themes with the same name then
     * the mode selects among those themes based on the operating system's dark/light mode.
     */
    theme_mode mode = theme_mode::light;

    theme() noexcept = default;
    theme(theme const&) noexcept = default;
    theme(theme&&) noexcept = default;
    theme& operator=(theme const&) noexcept = default;
    theme& operator=(theme&&) noexcept = default;

    /** Open and parse a theme file.
     */
    theme(hi::font_book const& font_book, std::filesystem::path const& url);

    /** Activate this theme.
     *
     * @post The global theme-values `hi::tv` have been overwritten with the values from this theme.
     */
    void activate() const noexcept;

private:
    container_type _items;

    void parse_data(hi::font_book const& font_book, datum const& data);
    void parse(hi::font_book const& font_book, datum const& data);
};

}} // namespace hi::v1

template<typename CharT>
struct std::formatter<hi::theme, CharT> : std::formatter<std::string, CharT> {
    auto format(hi::theme const& t, auto& fc)
    {
        return std::formatter<std::string_view, CharT>::format(std::format("{}:{}", t.name, t.mode), fc);
    }
};
