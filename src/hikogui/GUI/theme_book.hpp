// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "theme_mode.hpp"
#include "theme.hpp"
#include <limits>
#include <vector>
#include <memory>
#include <filesystem>

namespace hi::inline v1 {
class font_book;

/** theme_book keeps track of multiple themes.
 *
 */
class theme_book {
public:
    ~theme_book();
    theme_book(theme_book const &) = delete;
    theme_book(theme_book &&) = delete;
    theme_book &operator=(theme_book const &) = delete;
    theme_book &operator=(theme_book &&) = delete;

    theme_book(hi::font_book const &font_book, std::vector<std::filesystem::path> const &theme_directories) noexcept;

    [[nodiscard]] std::vector<std::string> theme_names() const noexcept;

    /** Find a theme matching the name and mode.
     *
     * @param name The name of the theme to select.
     * @param mode The mode of the theme to select.
     * @return A theme most closely matching the requested theme.
     */
    [[nodiscard]] theme const &find(std::string name, theme_mode mode) const noexcept;

private:
    std::vector<std::unique_ptr<theme>> themes;
};

} // namespace hi::inline v1
