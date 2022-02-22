// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "os_settings.hpp"
#include "log.hpp"

namespace tt::inline v1 {

[[nodiscard]] os_settings *os_settings::subsystem_init() noexcept
{
    return new os_settings();
}

void os_settings::subsystem_deinit() noexcept
{
    if (ttlet self = _global.exchange(nullptr)) {
        delete self;
    }
}

os_settings::~os_settings()
{
    timer::global().remove_callback(_gather_callback);
}

os_settings::os_settings() noexcept
{
    using namespace std::literals::chrono_literals;

    _gather_callback = timer::global().add_callback(
        gather_interval,
        [this](auto...) {
            this->_gather();
        },
        true);
}

void os_settings::_gather() noexcept
{
    using namespace std::literals::chrono_literals;

    ttlet lock = std::scoped_lock(_mutex);
    auto setting_has_changed = false;

    ttlet current_time = std::chrono::utc_clock::now();
    if (current_time < _gather_last_time + gather_minimum_interval) {
        return;
    }
    _gather_last_time = current_time;

    if (compare_store(_language_tags, gather_languages())) {
        setting_has_changed = true;
        _languages = language::make_languages(_language_tags);
        tt_log_info("OS language order has changed: {}", _languages);
    }

    if (compare_store(_theme_mode, gather_theme_mode())) {
        setting_has_changed = true;
        tt_log_info("OS theme-mode has changed: {}", _theme_mode.load());
    }

    if (compare_store(_double_click_interval, gather_double_click_interval())) {
        setting_has_changed = true;
        tt_log_info("OS double click interval has changed: {}", _double_click_interval.load());
    }

    if (compare_store(_keyboard_repeat_delay, gather_keyboard_repeat_delay())) {
        setting_has_changed = true;
        tt_log_info("OS keyboard repeat delay has changed: {}", _keyboard_repeat_delay.load());
    }

    if (compare_store(_keyboard_repeat_interval, gather_keyboard_repeat_interval())) {
        setting_has_changed = true;
        tt_log_info("OS keyboard repeat interval has changed: {}", _keyboard_repeat_interval.load());
    }

    if (compare_store(_cursor_blink_interval, gather_cursor_blink_interval())) {
        setting_has_changed = true;
        if (_cursor_blink_interval.load() < 1min) {
            tt_log_info("OS cursor blink interval has changed: {}", _cursor_blink_interval.load());
        } else {
            tt_log_info("OS cursor blink interval has changed: no-blinking");
        }
    }

    if (compare_store(_cursor_blink_delay, gather_cursor_blink_delay())) {
        setting_has_changed = true;
        tt_log_info("OS cursor blink delay has changed: {}", _cursor_blink_delay.load());
    }

    if (compare_store(_minimum_window_size, gather_minimum_window_size())) {
        setting_has_changed = true;
        tt_log_info("OS minimum window size has changed: {}", _minimum_window_size);
    }
    if (compare_store(_maximum_window_size, gather_maximum_window_size())) {
        setting_has_changed = true;
        tt_log_info("OS maximum window size has changed: {}", _maximum_window_size);
    }

    if (setting_has_changed) {
        _notifier();
    }
}

} // namespace tt::inline v1
