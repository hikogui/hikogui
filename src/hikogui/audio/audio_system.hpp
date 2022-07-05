// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_device.hpp"
#include "../unfair_recursive_mutex.hpp"
#include "../generator.hpp"
#include <vector>
#include <memory>

namespace hi::inline v1 {

/*! An system of audio devices.
 * Systems are for example: Window Audio Session API (WASAPI), ASIO, Apple CoreAudio
 */
class audio_system {
public:
    using token_type = notifier<>::token_type;

    /** Create an audio system object specific for the current operating system.
     */
    [[nodiscard]] static std::unique_ptr<audio_system> make_unique() noexcept;

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
    [[nodiscard]] virtual generator<audio_device *> devices() noexcept = 0;

    /** Subscribe a function to be called when the device list changes.
     *
     * @return A callback token, a RAII object which when destroyed removes the subscription.
     */
    token_type subscribe(callback_flags flags, std::invocable<> auto&& func) noexcept
    {
        return _notifier.subscribe(flags, hi_forward(func));
    }

    /** Subscribe a function to be called when the device list changes.
     *
     * @return A callback token, a RAII object which when destroyed removes the subscription.
     */
    token_type subscribe(std::invocable<> auto&& func) noexcept
    {
        return subscribe(callback_flags::synchronous, hi_forward(func));
    }

    auto operator co_await() noexcept
    {
        return _notifier.operator co_await();
    }

protected:
    notifier<> _notifier;
};

} // namespace hi::inline v1