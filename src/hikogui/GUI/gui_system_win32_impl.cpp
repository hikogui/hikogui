// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_system_win32.hpp"
#include "keyboard_bindings.hpp"
#include "theme_book.hpp"
#include "../GFX/module.hpp"
#include "../font/font.hpp"
#include "../path/path.hpp"
#include "../telemetry/module.hpp"
#include "../settings/settings.hpp"
#include "../macros.hpp"
#include <memory>

namespace hi::inline v1 {

[[nodiscard]] std::unique_ptr<gui_system> gui_system::make_unique(std::weak_ptr<gui_system_delegate> delegate) noexcept
{
    if (not os_settings::start_subsystem()) {
        hi_log_fatal("Could not start the os_settings subsystem.");
    }

    register_font_file(URL{"resource:fonts/elusiveicons-webfont.ttf"});
    register_font_file(URL{"resource:fonts/hikogui_icons.ttf"});
    register_font_directories(get_paths(path_location::font_dirs));

    auto theme_directories = make_vector(get_paths(path_location::theme_dirs));
    auto theme_book = std::make_unique<hi::theme_book>(std::move(theme_directories));

    auto gfx_system = std::make_unique<hi::gfx_system_vulkan>();

    auto keyboard_bindings = std::make_unique<hi::keyboard_bindings>();
    try {
        keyboard_bindings->load_bindings(URL{"resource:win32.keybinds.json"}, true);
    } catch (std::exception const &e) {
        hi_log_fatal("Could not load keyboard bindings. \"{}\"", e.what());
    }

    auto r = std::make_unique<gui_system_win32>(
        std::move(gfx_system),
        std::move(theme_book),
        std::move(keyboard_bindings),
        std::move(delegate));
    r->init();
    return r;
}

gui_system_win32::gui_system_win32(
    std::unique_ptr<gfx_system> gfx,
    std::unique_ptr<hi::theme_book> theme_book,
    std::unique_ptr<hi::keyboard_bindings> keyboard_bindings,
    std::weak_ptr<gui_system_delegate> delegate) :
    gui_system(
        std::move(gfx),
        std::move(theme_book),
        std::move(keyboard_bindings),
        std::move(delegate))
{
}

} // namespace hi::inline v1
