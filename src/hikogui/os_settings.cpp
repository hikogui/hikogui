// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "os_settings.hpp"
#include "log.hpp"

namespace hi::inline v1 {

[[nodiscard]] bool os_settings::subsystem_init() noexcept
{
    _gather_cbt = loop::timer().repeat_function(gather_interval, [] {
        os_settings::gather();
    });

    return true;
}

void os_settings::subsystem_deinit() noexcept
{
    if (_started.exchange(false)) {
        _gather_cbt = nullptr;
    }
}

void os_settings::gather() noexcept
{
    hilet lock = std::scoped_lock(_mutex);
    auto setting_has_changed = false;

    hilet current_time = std::chrono::utc_clock::now();
    if (current_time < _gather_last_time + gather_minimum_interval) {
        return;
    }
    _gather_last_time = current_time;

    try {
        auto language_tags = gather_languages();
        if (language_tags.empty()) {
            // If not language is configured on the system, use English as default.
            language_tags.emplace_back("en");
        }

        auto writing_direction = language_tags.front().writing_direction();

        auto languages = language::make_languages(language_tags);

        auto language_changed = compare_store(_language_tags, language_tags);
        language_changed |= compare_store(_languages, languages);
        language_changed |= compare_store(_writing_direction, writing_direction);

        if (language_changed) {
            setting_has_changed = true;
            hi_log_info("OS language order has changed: {}", _languages);
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS language: {}", e.what());
    }

    try {
        if (compare_store(_theme_mode, gather_theme_mode())) {
            setting_has_changed = true;
            hi_log_info("OS theme-mode has changed: {}", _theme_mode.load());
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS theme-mode: {}", e.what());
    }

    try {
        if (compare_store(_subpixel_orientation, gather_subpixel_orientation())) {
            setting_has_changed = true;
            hi_log_info("OS sub-pixel orientation has changed: {}", _subpixel_orientation.load());
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS sub-pixel orientation: {}", e.what());
    }

    try {
        if (compare_store(_uniform_HDR, gather_uniform_HDR())) {
            setting_has_changed = true;
            hi_log_info("OS uniform-HDR has changed: {}", _uniform_HDR.load());
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS uniform-HDR: {}", e.what());
    }

    try {
        if (compare_store(_double_click_interval, gather_double_click_interval())) {
            setting_has_changed = true;
            hi_log_info("OS double click interval has changed: {}", _double_click_interval.load());
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS double click interval: {}", e.what());
    }

    try {
        if (compare_store(_double_click_distance, gather_double_click_distance())) {
            setting_has_changed = true;
            hi_log_info("OS double click distance has changed: {}", _double_click_distance.load());
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS double click distance: {}", e.what());
    }

    try {
        if (compare_store(_keyboard_repeat_delay, gather_keyboard_repeat_delay())) {
            setting_has_changed = true;
            hi_log_info("OS keyboard repeat delay has changed: {}", _keyboard_repeat_delay.load());
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS keyboard repeat delay: {}", e.what());
    }

    try {
        if (compare_store(_keyboard_repeat_interval, gather_keyboard_repeat_interval())) {
            setting_has_changed = true;
            hi_log_info("OS keyboard repeat interval has changed: {}", _keyboard_repeat_interval.load());
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS keyboard repeat interval: {}", e.what());
    }

    try {
        if (compare_store(_cursor_blink_interval, gather_cursor_blink_interval())) {
            setting_has_changed = true;
            if (_cursor_blink_interval.load() < std::chrono::minutes(1)) {
                hi_log_info("OS cursor blink interval has changed: {}", _cursor_blink_interval.load());
            } else {
                hi_log_info("OS cursor blink interval has changed: no-blinking");
            }
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS cursor blink interval: {}", e.what());
    }

    try {
        if (compare_store(_cursor_blink_delay, gather_cursor_blink_delay())) {
            setting_has_changed = true;
            hi_log_info("OS cursor blink delay has changed: {}", _cursor_blink_delay.load());
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS cursor blink delay: {}", e.what());
    }

    try {
        if (compare_store(_minimum_window_width, gather_minimum_window_width())) {
            setting_has_changed = true;
            hi_log_info("OS minimum window width has changed: {}", _minimum_window_width.load());
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS minimum window width: {}", e.what());
    }

    try {
        if (compare_store(_minimum_window_height, gather_minimum_window_height())) {
            setting_has_changed = true;
            hi_log_info("OS minimum window height has changed: {}", _minimum_window_height.load());
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS minimum window height: {}", e.what());
    }

    try {
        if (compare_store(_maximum_window_width, gather_maximum_window_width())) {
            setting_has_changed = true;
            hi_log_info("OS maximum window width has changed: {}", _maximum_window_width.load());
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS maximum window width: {}", e.what());
    }

    try {
        if (compare_store(_maximum_window_height, gather_maximum_window_height())) {
            setting_has_changed = true;
            hi_log_info("OS maximum window height has changed: {}", _maximum_window_height.load());
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS maximum window height: {}", e.what());
    }

    try {
        if (compare_store(_primary_monitor_id, gather_primary_monitor_id())) {
            setting_has_changed = true;
            hi_log_info("OS primary monitor id has changed: {}", _primary_monitor_id.load());
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS primary monitor id: {}", e.what());
    }

    try {
        if (compare_store(_primary_monitor_rectangle, gather_primary_monitor_rectangle())) {
            setting_has_changed = true;
            hi_log_info("OS primary monitor rectangle has changed: {}", _primary_monitor_rectangle);
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS primary monitor rectangle: {}", e.what());
    }

    try {
        if (compare_store(_desktop_rectangle, gather_desktop_rectangle())) {
            setting_has_changed = true;
            hi_log_info("OS desktop rectangle has changed: {}", _desktop_rectangle);
        }
    } catch (std::exception const& e) {
        hi_log_error("Failed to get OS desktop rectangle: {}", e.what());
    }

    _populated.store(true, std::memory_order::release);
    if (setting_has_changed) {
        _notifier();
    }
}

} // namespace hi::inline v1
