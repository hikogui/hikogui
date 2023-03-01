// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_window.hpp"
#include "gui_window_win32.hpp"
#include "gui_system_delegate.hpp"
#include "../unicode/unicode_bidi_class.hpp"
#include "../GFX/gfx_device.hpp"
#include "../utility/module.hpp"
#include "../observer.hpp"
#include <span>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace hi::inline v1 {
class gfx_system;
class vertical_sync;
class keyboard_bindings;

/** Graphics system
 */
class gui_system {
public:
    static inline os_handle instance;

    std::unique_ptr<gfx_system> gfx;
    std::unique_ptr<hi::keyboard_bindings> keyboard_bindings;

    thread_id const thread_id;

    /** The name of the selected theme.
     */
    observer<std::string> selected_theme = "default";

    /** Make a gui_system instance.
     *
     * This will instantiate a gui_system instance appropriate for the current
     * operating system.
     *
     * @param delegate An optional delegate.
     * @return A unique pointer to a gui_system instance.
     */
    [[nodiscard]] static std::unique_ptr<gui_system> make_unique(std::weak_ptr<gui_system_delegate> delegate = {}) noexcept;

    virtual ~gui_system();

    gui_system(const gui_system&) = delete;
    gui_system& operator=(const gui_system&) = delete;
    gui_system(gui_system&&) = delete;
    gui_system& operator=(gui_system&&) = delete;

    void set_delegate(std::weak_ptr<gui_system_delegate> delegate) noexcept
    {
        _delegate = std::move(delegate);
    }

    std::shared_ptr<gui_window> add_window(std::shared_ptr<gui_window> window);

    /** Create a new window with an embedded widget.
     *
     * @tparam WidgetType The widget to construct in the window.
     * @param label The label for the window. The label is also passed to the
     *              widget.
     * @param args The arguments that are forwarded to the constructor of
     *             the window's widget.
     * @return A shared_ptr to the new window and a reference to the widget.
     */
    template<typename WidgetType, typename... Args>
    std::pair<std::shared_ptr<gui_window>, WidgetType&> make_window(hi::label const& label, Args&&...args)
    {
        hi_axiom(loop::main().on_thread());

        auto widget = std::make_unique<WidgetType>(label, std::forward<Args>(args)...);
        auto widget_ptr = widget.get();

        // XXX abstract away the _win32 part.
        auto window = std::make_shared<gui_window_win32>(*this, std::move(widget), label);

        return {add_window(std::move(window)), *widget_ptr};
    }

    /** Request all windows to constrain.
     */
    void request_reconstrain() noexcept;

protected:
    std::weak_ptr<gui_system_delegate> _delegate;

    gui_system(
        std::unique_ptr<gfx_system> gfx,
        std::unique_ptr<hi::keyboard_bindings> keyboard_bindings,
        std::weak_ptr<gui_system_delegate> delegate = {}) noexcept;

private:
    decltype(selected_theme)::callback_token _selected_theme_cbt;
    os_settings::callback_token _os_settings_cbt;
};

} // namespace hi::inline v1
