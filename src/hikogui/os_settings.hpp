// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "i18n/language_tag.hpp"
#include "i18n/language.hpp"
#include "GUI/theme_mode.hpp"
#include "GFX/subpixel_orientation.hpp"
#include "geometry/module.hpp"
#include "utility/module.hpp"
#include "loop.hpp"
#include "notifier.hpp"
#include <vector>
#include <mutex>

namespace hi::inline v1 {

class os_settings {
public:
    using notifier_type = notifier<>;
    using callback_token = notifier_type::callback_token;
    using callback_proto = notifier_type::callback_proto;

    /** Get the language tags for the configured languages.
     *
     * @return A list of language tags in order of priority.
     */
    [[nodiscard]] static std::vector<language_tag> language_tags() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        hilet lock = std::scoped_lock(_mutex);
        return _language_tags;
    }

    /** Get the configured languages.
     *
     * @note The list of languages include both the configured region-specific-languages and the generic-languages.
     * @return A list of languages in order of priority.
     */
    [[nodiscard]] static std::vector<language *> languages() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        hilet lock = std::scoped_lock(_mutex);
        return _languages;
    }

    /** Get the configured writing direction.
     *
     * The writing-direction is extracted from the first language/script configured on the system.
     *
     * @return Either `unicode_bidi_class::L` for left-to-right; or `unicode_bidi_class::R` for right-to-left.
     */
    [[nodiscard]] static unicode_bidi_class writing_direction() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        return _writing_direction.load(std::memory_order::relaxed);
    }

    /** Check if the configured writing direction is left-to-right.
     *
     * @retval true If the writing direction is left-to-right.
     */
    [[nodiscard]] static bool left_to_right() noexcept
    {
        return writing_direction() == unicode_bidi_class::L;
    }

    /** Check if the configured writing direction is right-to-left.
     *
     * @retval true If the writing direction is right-to-left.
     */
    [[nodiscard]] static bool right_to_left() noexcept
    {
        return not left_to_right();
    }

    /** Get the configured light/dark theme mode
     */
    [[nodiscard]] static hi::theme_mode theme_mode() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        return _theme_mode.load(std::memory_order::relaxed);
    }

    /** Get the configured light/dark theme mode
     */
    [[nodiscard]] static hi::subpixel_orientation subpixel_orientation() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        return _subpixel_orientation.load(std::memory_order::relaxed);
    }

    /** Whether SDR and HDR application can coexists on the same display.
     *
     * Microsoft Windows 10 and at least early versions of Windows 11 will
     * have set this to false, because if an application opens a HDR surface
     * it will switch the display mode to HDR, this switching may cause a
     * significant change in color and brightness of the display, including
     * other (SDR) applications that where already running. This would be
     * surprising for most users and we can not expect users to have calibrated
     * colors to match HDR with SDR.
     */
    [[nodiscard]] static bool uniform_HDR() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        return _uniform_HDR.load(std::memory_order::relaxed);
    }

    /** Get the mouse double click interval.
     */
    [[nodiscard]] static std::chrono::milliseconds double_click_interval() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        return _double_click_interval.load(std::memory_order::relaxed);
    }

    /** Get the distance from the previous mouse position to detect double click.
     */
    [[nodiscard]] static float double_click_distance() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        return _double_click_distance.load(std::memory_order::relaxed);
    }

    /** Get the delay before the keyboard starts repeating.
     *
     * @note Also used to determine the scroll delay when selecting text.
     */
    [[nodiscard]] static std::chrono::milliseconds keyboard_repeat_delay() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        return _keyboard_repeat_delay.load(std::memory_order::relaxed);
    }

    /** Get the keyboard repeat interval
     *
     * @note Also used to determine the scroll speed when selecting text.
     */
    [[nodiscard]] static std::chrono::milliseconds keyboard_repeat_interval() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        return _keyboard_repeat_interval.load(std::memory_order::relaxed);
    }

    /** Get the cursor blink delay.
     *
     * @note This delay is used to determine when to blink after cursor movement.
     */
    [[nodiscard]] static std::chrono::milliseconds cursor_blink_delay() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        return _cursor_blink_delay.load(std::memory_order::relaxed);
    }

    /** Get the cursor blink interval.
     *
     * @note The interval is the complete period of the cursor blink, from on-to-on.
     * @return The cursor blink interval, or `std::chrono::milliseconds::max()` when blinking is turned off.
     */
    [[nodiscard]] static std::chrono::milliseconds cursor_blink_interval() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        return _cursor_blink_interval.load(std::memory_order::relaxed);
    }

    /** The minimum width a window is allowed to be.
     *
     * @return The minimum window width.
     */
    [[nodiscard]] static int minimum_window_width() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        return _minimum_window_width.load(std::memory_order::relaxed);
    }

    /** The minimum height a window is allowed to be.
     *
     * @return The minimum window height.
     */
    [[nodiscard]] static int minimum_window_height() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        return _minimum_window_height.load(std::memory_order::relaxed);
    }

    /** The maximum width a window is allowed to be.
     *
     * @return The maximum window width.
     */
    [[nodiscard]] static int maximum_window_width() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        return _maximum_window_width.load(std::memory_order::relaxed);
    }

    /** The maximum height a window is allowed to be.
     *
     * @return The maximum window height.
     */
    [[nodiscard]] static int maximum_window_height() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        return _maximum_window_height.load(std::memory_order::relaxed);
    }

    /** Get the rectangle of the primary monitor.
     *
     * @return The rectangle describing the size and location inside the desktop.
     */
    [[nodiscard]] static aarectanglei primary_monitor_rectangle() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        hilet lock = std::scoped_lock(_mutex);
        return _primary_monitor_rectangle;
    }

    /** Get an opaque id of the primary monitor.
     */
    [[nodiscard]] static uintptr_t primary_monitor_id() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        return _primary_monitor_id.load(std::memory_order::relaxed);
    }

    /** Get the rectangle describing the desktop.
     *
     * @return The bounding rectangle around the desktop. With the origin being equal to the origin of the primary monitor.
     */
    [[nodiscard]] static aarectanglei desktop_rectangle() noexcept
    {
        hi_axiom(_populated.load(std::memory_order::acquire));
        hilet lock = std::scoped_lock(_mutex);
        return _desktop_rectangle;
    }

    /** Gather the settings from the operating system now.
     */
    static void gather() noexcept;

    [[nodiscard]] static callback_token
    subscribe(forward_of<callback_proto> auto&& callback, callback_flags flags = callback_flags::synchronous) noexcept
    {
        hilet lock = std::scoped_lock(_mutex);
        return _notifier.subscribe(hi_forward(callback), flags);
    }

    /** Get the global os_settings instance.
     *
     * @return True on success.
     */
    static bool start_subsystem() noexcept
    {
        return hi::start_subsystem(_started, false, subsystem_init, subsystem_deinit);
    }

private:
    static constexpr std::chrono::duration gather_interval = std::chrono::seconds(5);
    static constexpr std::chrono::duration gather_minimum_interval = std::chrono::seconds(1);

    static inline std::atomic<bool> _started = false;
    static inline std::atomic<bool> _populated = false;
    static inline unfair_mutex _mutex;
    static inline loop::timer_callback_token _gather_cbt;
    static inline utc_nanoseconds _gather_last_time;

    static inline notifier_type _notifier;

    static inline std::vector<language_tag> _language_tags = {};
    static inline std::vector<language *> _languages = {};
    static inline std::atomic<hi::unicode_bidi_class> _writing_direction = hi::unicode_bidi_class::L;
    static inline std::atomic<hi::theme_mode> _theme_mode = theme_mode::dark;
    static inline std::atomic<bool> _uniform_HDR = false;
    static inline std::atomic<hi::subpixel_orientation> _subpixel_orientation = hi::subpixel_orientation::unknown;
    static inline std::atomic<std::chrono::milliseconds> _double_click_interval = std::chrono::milliseconds(500);
    static inline std::atomic<float> _double_click_distance = 4.0f;
    static inline std::atomic<std::chrono::milliseconds> _keyboard_repeat_delay = std::chrono::milliseconds(250);
    static inline std::atomic<std::chrono::milliseconds> _keyboard_repeat_interval = std::chrono::milliseconds(33);
    static inline std::atomic<std::chrono::milliseconds> _cursor_blink_interval = std::chrono::milliseconds(1000);
    static inline std::atomic<std::chrono::milliseconds> _cursor_blink_delay = std::chrono::milliseconds(1000);
    static inline std::atomic<int> _minimum_window_width = 40;
    static inline std::atomic<int> _minimum_window_height = 25;
    static inline std::atomic<int> _maximum_window_width = 1920;
    static inline std::atomic<int> _maximum_window_height = 1080;
    static inline std::atomic<uintptr_t> _primary_monitor_id = 0;
    static inline aarectanglei _primary_monitor_rectangle = aarectanglei{0, 0, 1920, 1080};
    static inline aarectanglei _desktop_rectangle = aarectanglei{0, 0, 1920, 1080};

    [[nodiscard]] static bool subsystem_init() noexcept;
    static void subsystem_deinit() noexcept;

    [[nodiscard]] static std::vector<language_tag> gather_languages();
    [[nodiscard]] static hi::theme_mode gather_theme_mode();
    [[nodiscard]] static hi::subpixel_orientation gather_subpixel_orientation();
    [[nodiscard]] static bool gather_uniform_HDR();
    [[nodiscard]] static std::chrono::milliseconds gather_double_click_interval();
    [[nodiscard]] static float gather_double_click_distance();
    [[nodiscard]] static std::chrono::milliseconds gather_keyboard_repeat_delay();
    [[nodiscard]] static std::chrono::milliseconds gather_keyboard_repeat_interval();
    [[nodiscard]] static std::chrono::milliseconds gather_cursor_blink_interval();
    [[nodiscard]] static std::chrono::milliseconds gather_cursor_blink_delay();
    [[nodiscard]] static int gather_minimum_window_width();
    [[nodiscard]] static int gather_minimum_window_height();
    [[nodiscard]] static int gather_maximum_window_width();
    [[nodiscard]] static int gather_maximum_window_height();
    [[nodiscard]] static uintptr_t gather_primary_monitor_id();
    [[nodiscard]] static aarectanglei gather_primary_monitor_rectangle();
    [[nodiscard]] static aarectanglei gather_desktop_rectangle();
};

} // namespace hi::inline v1
