// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_window.hpp"
#include "gui_window_win32.hpp"
#include "gui_system_delegate.hpp"
#include "../unicode/unicode_bidi_class.hpp"
#include "../GFX/gfx_device.hpp"
#include "../thread.hpp"
#include "../unfair_recursive_mutex.hpp"
#include "../observer.hpp"
#include <span>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace hi::inline v1 {
class gfx_system;
class vertical_sync;
class font_book;
class theme_book;
class keyboard_bindings;

/** Graphics system
 */
class gui_system {
public:
    static inline os_handle instance;

    std::unique_ptr<gfx_system> gfx;
    std::unique_ptr<hi::font_book> font_book;
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

    gui_system(const gui_system &) = delete;
    gui_system &operator=(const gui_system &) = delete;
    gui_system(gui_system &&) = delete;
    gui_system &operator=(gui_system &&) = delete;

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

    /** Create a new window.
     * @param args The arguments that are forwarded to the constructor of
     *             `hi::gui_window_win32`.
     * @return A reference to the new window.
     */
    template<typename... Args>
    std::shared_ptr<gui_window> make_window(Args &&...args)
    {
        hi_axiom(is_gui_thread());

        // XXX abstract away the _win32 part.
        auto window = std::make_shared<gui_window_win32>(*this, std::forward<Args>(args)...);
        window->init();

        return add_window(std::move(window));
    }

    /** Check if this thread is the same as the gui thread.
     */
    [[nodiscard]] bool is_gui_thread() const noexcept
    {
        return thread_id == current_thread_id();
    }

    /** Request all windows to constrain.
     */
    void request_reconstrain() noexcept;

protected:
    gui_system(
        std::unique_ptr<gfx_system> gfx,
        std::unique_ptr<hi::font_book> font_book,
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
