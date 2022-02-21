// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text/language_tag.hpp"
#include "text/language.hpp"
#include "GUI/theme_mode.hpp"
#include "unfair_mutex.hpp"
#include "subsystem.hpp"
#include "timer.hpp"
#include "notifier.hpp"
#include <vector>
#include <mutex>

namespace tt::inline v1 {

class os_settings {
public:
    using callback_ptr_type = notifier<void()>::callback_ptr_type;

    /** Get the language tags for the configured languages.
     *
     * @return A list of language tags in order of priority.
     */
    [[nodiscard]] static std::vector<language_tag> language_tags() noexcept
    {
        if (ttlet self = global()) {
            ttlet lock = std::scoped_lock(self->_mutex);
            return self->_language_tags;
        } else {
            return {};
        }
    }

    /** Get the configured languages.
     *
     * @note The list of languages include both the configured region-specific-languages and the generic-languages.
     * @return A list of languages in order of priority.
     */
    [[nodiscard]] static std::vector<language *> languages() noexcept
    {
        if (ttlet self = global()) {
            ttlet lock = std::scoped_lock(self->_mutex);
            return self->_languages;
        } else {
            return {};
        }
    }

    /** Get the configured light/dark theme mode
     */
    [[nodiscard]] static tt::theme_mode theme_mode() noexcept
    {
        if (ttlet self = global()) {
            return self->_theme_mode.load(std::memory_order_relaxed);
        } else {
            return theme_mode::dark;
        }
    }

    /** Get the mouse double click interval.
     */
    [[nodiscard]] static std::chrono::milliseconds double_click_interval() noexcept
    {
        if (ttlet self = global()) {
            return self->_double_click_interval.load(std::memory_order_relaxed);
        } else {
            return std::chrono::milliseconds{500};
        }
    }

    /** Get the delay before the keyboard starts repeating.
     *
     * @note Also used to determine the scroll delay when selecting text.
     */
    [[nodiscard]] static std::chrono::milliseconds keyboard_repeat_delay() noexcept
    {
        if (ttlet self = global()) {
            return self->_keyboard_repeat_delay.load(std::memory_order_relaxed);
        } else {
            return std::chrono::milliseconds{100};
        }
    }

    /** Get the keyboard repeat interval
     *
     * @note Also used to determine the scroll speed when selecting text.
     */
    [[nodiscard]] static std::chrono::milliseconds keyboard_repeat_interval() noexcept
    {
        if (ttlet self = global()) {
            return self->_keyboard_repeat_interval.load(std::memory_order_relaxed);
        } else {
            return std::chrono::milliseconds{250};
        }
    }

    /** Get the cursor blink delay.
     *
     * @note This delay is used to determine when to blink after cursor movement.
     */
    [[nodiscard]] static std::chrono::milliseconds cursor_blink_delay() noexcept
    {
        if (ttlet self = global()) {
            return self->_cursor_blink_delay.load(std::memory_order_relaxed);
        } else {
            return std::chrono::milliseconds{500};
        }
    }

    /** Get the cursor blink interval.
     *
     * @note The interval is the complete period of the cursor blink, from on-to-on.
     * @return The cursor blink interval, or `std::chrono::milliseconds::max()` when blinking is turned off.
     */
    [[nodiscard]] static std::chrono::milliseconds cursor_blink_interval() noexcept
    {
        if (ttlet self = global()) {
            return self->_cursor_blink_interval.load(std::memory_order_relaxed);
        } else {
            return std::chrono::milliseconds{500};
        }
    }

    /** Gather the settings from the operating system now.
     */
    static void gather() noexcept
    {
        if (ttlet self = global()) {
            return self->_gather();
        }
    }

    [[nodiscard]] static callback_ptr_type subscribe(callback_ptr_type const &callback) noexcept
    {
        if (ttlet self = global()) {
            ttlet lock = std::scoped_lock(self->_mutex);
            return self->_notifier.subscribe(callback);
        } else {
            return callback;
        }
    }

    template<typename Callback>
    [[nodiscard]] static callback_ptr_type subscribe(Callback &&callback) noexcept requires(std::is_invocable_v<Callback>)
    {
        if (ttlet self = global()) {
            ttlet lock = std::scoped_lock(self->_mutex);
            return self->_notifier.subscribe(std::forward<Callback>(callback));
        } else {
            return nullptr;
        }
    }

    static void unsubscribe(callback_ptr_type const &callback) noexcept
    {
        if (ttlet self = global()) {
            ttlet lock = std::scoped_lock(self->_mutex);
            self->_notifier.unsubscribe(callback);
        }
    }

private:
    static constexpr std::chrono::duration gather_interval = std::chrono::seconds(5);
    static constexpr std::chrono::duration gather_minimum_interval = std::chrono::seconds(1);
    static inline std::atomic<os_settings *> _global = nullptr;

    mutable unfair_mutex _mutex;
    timer::callback_ptr_type _gather_callback;
    utc_nanoseconds _gather_last_time;

    notifier<void()> _notifier;

    std::vector<language_tag> _language_tags = {};
    std::vector<language *> _languages = {};
    std::atomic<tt::theme_mode> _theme_mode = theme_mode::dark;
    std::atomic<std::chrono::milliseconds> _double_click_interval = {};
    std::atomic<std::chrono::milliseconds> _keyboard_repeat_delay = {};
    std::atomic<std::chrono::milliseconds> _keyboard_repeat_interval = {};
    std::atomic<std::chrono::milliseconds> _cursor_blink_interval = {};
    std::atomic<std::chrono::milliseconds> _cursor_blink_delay = {};

    /** Get the global os_settings instance.
     *
     * @return The global os_settings instance or nullptr during shutdown.
     */
    [[nodiscard]] static os_settings *global() noexcept
    {
        return start_subsystem(_global, nullptr, subsystem_init, subsystem_deinit);
    }

    [[nodiscard]] static os_settings *subsystem_init() noexcept;
    static void subsystem_deinit() noexcept;

    ~os_settings();
    os_settings() noexcept;
    os_settings(os_settings const &) = delete;
    os_settings(os_settings &&) = delete;
    os_settings &operator=(os_settings const &) = delete;
    os_settings &operator=(os_settings &&) = delete;

    void _gather() noexcept;
    [[nodiscard]] static std::vector<language_tag> gather_languages() noexcept;
    [[nodiscard]] static tt::theme_mode gather_theme_mode() noexcept;
    [[nodiscard]] static std::chrono::milliseconds gather_double_click_interval() noexcept;
    [[nodiscard]] std::chrono::milliseconds gather_keyboard_repeat_delay() noexcept;
    [[nodiscard]] std::chrono::milliseconds gather_keyboard_repeat_interval() noexcept;
    [[nodiscard]] std::chrono::milliseconds gather_cursor_blink_interval() noexcept;
    [[nodiscard]] std::chrono::milliseconds gather_cursor_blink_delay() noexcept;
};

} // namespace tt::inline v1
