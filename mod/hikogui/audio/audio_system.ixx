// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <vector>
#include <memory>
#include <concepts>
#include <coroutine>

export module hikogui_audio_audio_system;
import hikogui_audio_audio_device;
import hikogui_coroutine;
import hikogui_utility;

export namespace hi { inline namespace v1 {

/*! An system of audio devices.
 * Systems are for example: Window Audio Session API (WASAPI), ASIO, Apple CoreAudio
 */
export class audio_system {
public:
    /** Create an audio system object specific for the current operating system.
     */
    [[nodiscard]] static audio_system &global() noexcept;

    audio_system() = default;
    virtual ~audio_system() = default;
    audio_system(audio_system const&) = delete;
    audio_system(audio_system&&) = delete;
    audio_system& operator=(audio_system const&) = delete;
    audio_system& operator=(audio_system&&) = delete;

    /** The devices that are part of the audio system.
     *
     * Due to complicated threading and callback function interactions
     * audio devices are not destroyed until application shutdown.
     *
     * @return A generator-coroutine object that can be iterated over.
     */
    [[nodiscard]] virtual generator<audio_device &> devices() noexcept = 0;

    /** Subscribe a function to be called when the device list changes.
     *
     * @return A callback token, a RAII object which when destroyed removes the subscription.
     */
    template<forward_of<void()> Func>
    [[nodiscard]] callback<void()> subscribe(Func&& func, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return _notifier.subscribe(std::forward<Func>(func), flags);
    }

    auto operator co_await() const noexcept
    {
        return _notifier.operator co_await();
    }

protected:
    notifier<void()> _notifier;
};

namespace detail {
std::unique_ptr<audio_system> audio_system_global;
}

template<typename Context>
concept audio_device_filter = std::same_as<Context, audio_device_state> or std::same_as<Context, audio_direction>;

[[nodiscard]] constexpr bool match_audio_device(audio_device const &device) noexcept
{
    return true;
}

template<audio_device_filter FirstFilter, audio_device_filter... Filters>
[[nodiscard]] bool match_audio_device(audio_device const &device, FirstFilter &&first_filter, Filters &&...filters) noexcept
{
    if constexpr (std::same_as<FirstFilter, audio_device_state>) {
        if (device.state() != first_filter) {
            return false;
        }
    } else if constexpr (std::same_as<FirstFilter, audio_direction>) {
        if (not to_bool(device.direction() & first_filter)) {
            return false;
        }
    } else {
        hi_static_no_default();
    }

    return match_audio_device(device, std::forward<Filters>(filters)...);
}

/** Get audio devices matching the filter arguments.
 * 
 * @param filters A list of filters that the audio devices need to match with.
 *                Filters are of the types `hi::audio_device_state` and
 *                `hi::audio_direction`.
 * @return A list of audio devices matching the filters.
 */
template<audio_device_filter... Filters>
[[nodiscard]] generator<audio_device &> audio_devices(Filters &&...filters) noexcept
{
    for (auto &device: audio_system::global().devices()) {
        if (match_audio_device(device, std::forward<Filters>(filters)...)) {
            co_yield device;
        }
    }
}

}} // namespace hi::inline v1
