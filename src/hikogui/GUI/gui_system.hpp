// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_window.hpp"
#include "gui_window_win32.hpp"
#include "gui_system_delegate.hpp"
#include "widget_intf.hpp"
#include "../unicode/unicode.hpp"
#include "../GFX/module.hpp"
#include "../utility/utility.hpp"
#include "../observer/module.hpp"
#include "../macros.hpp"
#include <span>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include <concepts>
#include <utility>



namespace hi::inline v1 {
class gfx_system;
class vertical_sync;
class theme_book;
class keyboard_bindings;

/** Graphics system
 */
class gui_system {
public:
    std::unique_ptr<gfx_system> gfx;
    std::unique_ptr<hi::theme_book> theme_book;
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

    /** Initialize after construction.
     * Call this function directly after the constructor on the same thread.
     */
    virtual void init() noexcept
    {
        if (auto delegate = _delegate.lock()) {
            delegate->init(*this);
        }
    }

    virtual void deinit() noexcept
    {
        if (auto delegate = _delegate.lock()) {
            delegate->deinit(*this);
        }
    }

    void set_delegate(std::weak_ptr<gui_system_delegate> delegate) noexcept
    {
        _delegate = std::move(delegate);
    }

    std::shared_ptr<gui_window> add_window(std::shared_ptr<gui_window> window);

    /** Create a new window with the specified managing widget.
     *
     * @param widget The widget that manages the window.
     * @return A owning pointer to the new window.
     *         releasing the pointer will close the window.
     */
    [[nodiscard]] std::shared_ptr<gui_window> make_window_with_widget(std::unique_ptr<widget_intf> widget)
    {
        hi_axiom(loop::main().on_thread());

        // XXX abstract away the _win32 part.
        return add_window(std::make_shared<gui_window_win32>(*this, std::move(widget)));
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
    [[nodiscard]] std::pair<std::shared_ptr<gui_window>, Widget&> make_window(Args&&...args)
    {
        auto widget = std::make_unique<Widget>(std::forward<Args>(args)...);
        auto& widget_ref = *widget;

        return {make_window_with_widget(std::move(widget)), widget_ref};
    }

    /** Request all windows to constrain.
     */
    void request_reconstrain() noexcept;

protected:
    gui_system(
        std::unique_ptr<gfx_system> gfx,
        std::unique_ptr<hi::theme_book> theme_book,
        std::unique_ptr<hi::keyboard_bindings> keyboard_bindings,
        std::weak_ptr<gui_system_delegate> delegate = {}) noexcept;

private:
    std::weak_ptr<gui_system_delegate> _delegate;

    /** The theme of the system.
     * Should never be nullptr in reality.
     */
    hi::theme const *_theme = nullptr;
};

} // namespace hi::inline v1
