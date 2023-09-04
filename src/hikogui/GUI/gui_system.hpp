// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_window.hpp"
#include "gui_window_win32.hpp"
#include "widget_intf.hpp"
#include "theme_book.hpp"
#include "keyboard_bindings.hpp"
#include "../unicode/unicode.hpp"
#include "../GFX/GFX.hpp"
#include "../utility/utility.hpp"
#include "../observer/module.hpp"
#include "../telemetry/telemetry.hpp"
#include "../font/font.hpp"
#include "../path/path.hpp"
#include "../macros.hpp"
#include <span>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <concepts>
#include <utility>

namespace hi::inline v1 {

/** Graphics system
 */
class gui_system {
public:
    thread_id const thread_id;

    /** The name of the selected theme.
     */
    observer<std::string> selected_theme = "default";

    virtual ~gui_system() = default;
    gui_system(const gui_system&) = delete;
    gui_system& operator=(const gui_system&) = delete;
    gui_system(gui_system&&) = delete;
    gui_system& operator=(gui_system&&) = delete;
    gui_system() noexcept : thread_id(current_thread_id()) {}

    [[nodiscard]] static gui_system &global() noexcept;

private:
    inline static std::unique_ptr<gui_system> _global;

    /** The theme of the system.
     * Should never be nullptr in reality.
     */
    hi::theme const *_theme = nullptr;
};

[[nodiscard]] inline gui_system &gui_system::global() noexcept
{
    if (not _global) {
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

        _global = std::make_unique<gui_system>();
    }
    return *_global;
}

inline void add_window(gui_window &window)
{
    hi_axiom(loop::main().on_thread());

    auto device = gfx_system::global().find_best_device_for_surface(window.surface->intrinsic);
    if (not device) {
        throw gui_error("Could not find a vulkan-device matching this window");
    }

    window.set_device(device);
}

/** Create a new window with the specified managing widget.
 *
 * @param widget The widget that manages the window.
 * @return A owning pointer to the new window.
 *         releasing the pointer will close the window.
 */
[[nodiscard]] inline std::unique_ptr<gui_window> make_unique_window_with_widget(std::unique_ptr<widget_intf> widget)
{
    auto window = std::make_unique<gui_window_win32>(std::move(widget));
    add_window(*window);
    return window;
}

/** Create a new window.
 *
 * @tparam Widget The type of widget to create to manage the window.
 * @param args The arguments that are forwarded to the constructor of the managing
 *             widget that is created.
 * @return A owning pointer to the new window.
 *         releasing the pointer will close the window.
 */
template<std::derived_from<widget_intf> Widget, typename... Args>
[[nodiscard]] inline std::pair<std::unique_ptr<gui_window>, Widget&> make_unique_window(Args&&...args)
{
    auto widget = std::make_unique<Widget>(std::forward<Args>(args)...);
    auto& widget_ref = *widget;

    return {make_unique_window_with_widget(std::move(widget)), widget_ref};
}

} // namespace hi::inline v1
