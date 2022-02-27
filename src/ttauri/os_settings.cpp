// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "os_settings.hpp"
#include "log.hpp"

namespace tt::inline v1 {

[[nodiscard]] bool os_settings::subsystem_init() noexcept
{
    _gather_callback = timer::global().add_callback(
        gather_interval,
        [](auto...) {
            os_settings::gather();
        },
        true);

    return true;
}

void os_settings::subsystem_deinit() noexcept
{
    if (_started.exchange(false)) {
        timer::global().remove_callback(_gather_callback);
    }
}

void os_settings::gather() noexcept
{
    ttlet lock = std::scoped_lock(_mutex);
    auto setting_has_changed = false;

    ttlet current_time = std::chrono::utc_clock::now();
    if (current_time < _gather_last_time + gather_minimum_interval) {
        return;
    }
    _gather_last_time = current_time;

    try {
        auto language_tags = gather_languages();
        auto languages = language::make_languages(language_tags);

        auto language_changed = compare_store(_language_tags, language_tags);
        language_changed |= compare_store(_languages, languages);

        if (language_changed) {
            setting_has_changed = true;
            tt_log_info("OS language order has changed: {}", _languages);
        }
    } catch (std::exception const &e) {
        tt_log_error("Failed to get OS language: {}", e.what());
    }

    try {
        if (compare_store(_theme_mode, gather_theme_mode())) {
            setting_has_changed = true;
            tt_log_info("OS theme-mode has changed: {}", _theme_mode.load());
        }
    } catch (std::exception const &e) {
        tt_log_error("Failed to get OS theme-mode: {}", e.what());
    }

    try {
        if (compare_store(_subpixel_orientation, gather_subpixel_orientation())) {
            setting_has_changed = true;
            tt_log_info("OS sub-pixel orientation has changed: {}", _subpixel_orientation.load());
        }
    } catch (std::exception const &e) {
        tt_log_error("Failed to get OS sub-pixel orientation: {}", e.what());
    }


    try {
        if (compare_store(_double_click_interval, gather_double_click_interval())) {
            setting_has_changed = true;
            tt_log_info("OS double click interval has changed: {}", _double_click_interval.load());
        }
    } catch (std::exception const &e) {
        tt_log_error("Failed to get OS double click interval: {}", e.what());
    }

    try {
        if (compare_store(_keyboard_repeat_delay, gather_keyboard_repeat_delay())) {
            setting_has_changed = true;
            tt_log_info("OS keyboard repeat delay has changed: {}", _keyboard_repeat_delay.load());
        }
    } catch (std::exception const &e) {
        tt_log_error("Failed to get OS keyboard repeat delay: {}", e.what());
    }

    try {
        if (compare_store(_keyboard_repeat_interval, gather_keyboard_repeat_interval())) {
            setting_has_changed = true;
            tt_log_info("OS keyboard repeat interval has changed: {}", _keyboard_repeat_interval.load());
        }
    } catch (std::exception const &e) {
        tt_log_error("Failed to get OS keyboard repeat interval: {}", e.what());
    }

    try {
        if (compare_store(_cursor_blink_interval, gather_cursor_blink_interval())) {
            setting_has_changed = true;
            if (_cursor_blink_interval.load() < std::chrono::minutes(1)) {
                tt_log_info("OS cursor blink interval has changed: {}", _cursor_blink_interval.load());
            } else {
                tt_log_info("OS cursor blink interval has changed: no-blinking");
            }
        }
    } catch (std::exception const &e) {
        tt_log_error("Failed to get OS cursor blink interval: {}", e.what());
    }

    try {
        if (compare_store(_cursor_blink_delay, gather_cursor_blink_delay())) {
            setting_has_changed = true;
            tt_log_info("OS cursor blink delay has changed: {}", _cursor_blink_delay.load());
        }
    } catch (std::exception const &e) {
        tt_log_error("Failed to get OS cursor blink delay: {}", e.what());
    }

    try {
        if (compare_store(_minimum_window_size, gather_minimum_window_size())) {
            setting_has_changed = true;
            tt_log_info("OS minimum window size has changed: {}", _minimum_window_size);
        }
    } catch (std::exception const &e) {
        tt_log_error("Failed to get OS minimum window size: {}", e.what());
    }

    try {
        if (compare_store(_maximum_window_size, gather_maximum_window_size())) {
            setting_has_changed = true;
            tt_log_info("OS maximum window size has changed: {}", _maximum_window_size);
        }
    } catch (std::exception const &e) {
        tt_log_error("Failed to get OS maximum window size: {}", e.what());
    }

    try {
        if (compare_store(_primary_monitor_rectangle, gather_primary_monitor_rectangle())) {
            setting_has_changed = true;
            tt_log_info("OS primary monitor rectangle has changed: {}", _primary_monitor_rectangle);
        }
    } catch (std::exception const &e) {
        tt_log_error("Failed to get OS primary monitor rectangle: {}", e.what());
    }

    try {
        if (compare_store(_desktop_rectangle, gather_desktop_rectangle())) {
            setting_has_changed = true;
            tt_log_info("OS desktop rectangle has changed: {}", _desktop_rectangle);
        }
    } catch (std::exception const &e) {
        tt_log_error("Failed to get OS desktop rectangle: {}", e.what());
    }

    if (setting_has_changed) {
        _notifier();
    }
}

} // namespace tt::inline v1
