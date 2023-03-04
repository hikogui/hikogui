// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gui_system_win32.hpp"
#include "keyboard_bindings.hpp"
#include "../theme/module.hpp"
#include "../GFX/module.hpp"
#include "../font/module.hpp"
#include "../file/path_location.hpp"
#include "../file/URL.hpp"
#include "../trace.hpp"
#include "../log.hpp"
#include "../os_settings.hpp"
#include <memory>

namespace hi::inline v1 {

[[nodiscard]] std::unique_ptr<gui_system> gui_system::make_unique(std::weak_ptr<gui_system_delegate> delegate) noexcept
{
    if (not os_settings::start_subsystem()) {
        hi_log_fatal("Could not start the os_settings subsystem.");
    }

    for (auto path : get_paths(path_location::font_dirs)) {
        register_font_directory(path);
    }
    register_font_file(URL{"resource:fonts/elusiveicons-webfont.ttf"});
    register_font_file(URL{"resource:fonts/hikogui_icons.ttf"});

    for (auto path : get_paths(path_location::theme_dirs)) {
        register_theme_directory(path);
    }
    // A theme will be activated when gui_system constructor is executed.

    auto gfx_system = std::make_unique<hi::gfx_system_vulkan>();

    auto keyboard_bindings = std::make_unique<hi::keyboard_bindings>();
    try {
        keyboard_bindings->load_bindings(URL{"resource:win32.keybinds.json"}, true);
    } catch (std::exception const& e) {
        hi_log_fatal("Could not load keyboard bindings. \"{}\"", e.what());
    }

    return std::make_unique<gui_system_win32>(std::move(gfx_system), std::move(keyboard_bindings), std::move(delegate));
}

gui_system_win32::~gui_system_win32()
{
    if (auto delegate_ = _delegate.lock()) {
        delegate_->deinit(*this);
    }
}

gui_system_win32::gui_system_win32(
    std::unique_ptr<gfx_system> gfx,
    std::unique_ptr<hi::keyboard_bindings> keyboard_bindings,
    std::weak_ptr<gui_system_delegate> delegate) :
    gui_system(std::move(gfx), std::move(keyboard_bindings), std::move(delegate))
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if (auto delegate_ = _delegate.lock()) {
        delegate_->init(*this);
    }
}

} // namespace hi::inline v1
