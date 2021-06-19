// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "theme_mode.hpp"
#include "theme.hpp"
#include <limits>
#include <vector>
#include <new>


namespace tt {

/** theme_book keeps track of multiple themes.
 * The theme_book is instantiated during application startup
 * 
 */
class theme_book {
public:
    ~theme_book();
    theme_book(theme_book const &) = delete;
    theme_book(theme_book &&) = delete;
    theme_book &operator=(theme_book const &) = delete;
    theme_book &operator=(theme_book &&) = delete;

    theme_book(std::vector<URL> const &theme_directories) noexcept;

    [[nodiscard]] std::vector<std::string> theme_names() const noexcept;

    [[nodiscard]] tt::theme_mode current_theme_mode() const noexcept;

    void set_current_theme_mode(tt::theme_mode theme_mode) noexcept;

    [[nodiscard]] std::string current_theme_name() const noexcept;

    void set_current_theme_name(std::string const &themeName) noexcept;

    static theme_book &global() noexcept
    {
        auto ptr = start_subsystem(_global, nullptr, subsystem_init, subsystem_deinit);
        tt_axiom(ptr);
        return *ptr;
    }

private:
    static inline std::atomic<theme_book *>_global = nullptr;

    std::vector<std::unique_ptr<theme>> themes;
    std::string _current_theme_name;
    tt::theme_mode _current_theme_mode;

    static inline char const *_default_theme_name = "TTauri";

    [[nodiscard]] static theme_book *subsystem_init() noexcept;
    static void subsystem_deinit() noexcept;

    /** Find a theme matching the current name and mode.
     */
    void update_theme() noexcept;
};

}
