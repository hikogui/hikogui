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

    [[nodiscard]] static std::vector<language_tag> language_tags() noexcept
    {
        if (ttlet self = global()) {
            ttlet lock = std::scoped_lock(self->_mutex);
            return self->_language_tags;
        } else {
            return {};
        }
    }

    [[nodiscard]] static std::vector<language *> languages() noexcept
    {
        if (ttlet self = global()) {
            ttlet lock = std::scoped_lock(self->_mutex);
            return self->_languages;
        } else {
            return {};
        }
    }

    [[nodiscard]] static tt::theme_mode theme_mode() noexcept
    {
        if (ttlet self = global()) {
            return self->_theme_mode.load(std::memory_order_relaxed);
        } else {
            return theme_mode::dark;
        }
    }

    [[nodiscard]] static std::chrono::milliseconds double_click_interval() noexcept
    {
        if (ttlet self = global()) {
            return self->_double_click_interval.load(std::memory_order_relaxed);
        } else {
            return std::chrono::milliseconds{500};
        }
    }

    [[nodiscard]] static std::chrono::milliseconds keyboard_repeat_delay() noexcept
    {
        if (ttlet self = global()) {
            return self->_keyboard_repeat_delay.load(std::memory_order_relaxed);
        } else {
            return std::chrono::milliseconds{100};
        }
    }

    [[nodiscard]] static std::chrono::milliseconds keyboard_repeat_interval() noexcept
    {
        if (ttlet self = global()) {
            return self->_keyboard_repeat_interval.load(std::memory_order_relaxed);
        } else {
            return std::chrono::milliseconds{250};
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
    std::atomic<std::chrono::milliseconds> _double_click_interval = std::chrono::milliseconds(500);
    std::atomic<std::chrono::milliseconds> _keyboard_repeat_delay = std::chrono::milliseconds(250);
    std::atomic<std::chrono::milliseconds> _keyboard_repeat_interval = std::chrono::milliseconds(100);

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
};

} // namespace tt::inline v1
