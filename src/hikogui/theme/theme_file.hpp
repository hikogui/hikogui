// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "theme_mode.hpp"
#include "../text/module.hpp"
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

/** A theme_file loaded from a theme_file file.
 *
 * The theme_file object is not directly used when drawing the user interface.
 * The draw function will use the theme_file-value API `hi::tv` to select specific values.
 *
 * When a theme_file is activated it will update the global `hi::tv` values.
 *
 * Loading of themes is done through the theme_file-book.
 */
class theme_file {
public:
    using value_type = std::variant<float, std::vector<hi::color>, std::vector<hi::text_style>, std::string>;
    using container_type = std::vector<std::pair<std::string, value_type>>;

    /** The name of the theme-file.
     *
     * The name may be repeated, once for each mode.
     */
    std::string name;

    /** The mode that this theme-file is used for.
     *
     * If there are multiple themes with the same name then
     * the mode selects among those themes based on the operating system's dark/light mode.
     */
    theme_mode mode = theme_mode::light;

    theme_file() noexcept = default;
    theme_file(theme_file const&) noexcept = default;
    theme_file(theme_file&&) noexcept = default;
    theme_file& operator=(theme_file const&) noexcept = default;
    theme_file& operator=(theme_file&&) noexcept = default;

    /** Open and parse a theme_file file.
     */
    theme_file(std::filesystem::path const& url);

    /** Activate this theme_file.
     *
     * @post The global theme_file-values `hi::tv` have been overwritten with the values from this theme_file.
     */
    void activate() const noexcept;

private:
    container_type _items;

    void parse_data(datum const& data);
    void parse(datum const& data);
};

}} // namespace hi::v1

template<typename CharT>
struct std::formatter<hi::theme_file, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::theme_file const& t, auto& fc)
    {
        return std::formatter<std::string_view, CharT>::format(std::format("{}:{}", t.name, t.mode), fc);
    }
};
