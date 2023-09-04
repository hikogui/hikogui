// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_system_win32.hpp"
#include "keyboard_bindings.hpp"
#include "theme_book.hpp"
#include "../GFX/GFX.hpp"
#include "../font/font.hpp"
#include "../path/path.hpp"
#include "../telemetry/telemetry.hpp"
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

    register_theme_directories(get_paths(path_location::theme_dirs));

    try {
        load_system_keyboard_bindings(URL{"resource:win32.keybinds.json"});
    } catch (std::exception const& e) {
        hi_log_fatal("Could not load keyboard bindings. \"{}\"", e.what());
    }

    auto r = std::make_unique<gui_system_win32>(std::move(delegate));
    r->init();
    return r;
}

gui_system_win32::gui_system_win32(std::weak_ptr<gui_system_delegate> delegate) :
    gui_system(std::move(delegate))
{
}

} // namespace hi::inline v1
