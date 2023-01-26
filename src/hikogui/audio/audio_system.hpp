// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_device.hpp"
#include "../utility/module.hpp"
#include "../generator.hpp"
#include <vector>
#include <memory>

namespace hi::inline v1 {

/*! An system of audio devices.
 * Systems are for example: Window Audio Session API (WASAPI), ASIO, Apple CoreAudio
 */
class audio_system {
public:
    using notifier_type = notifier<>;
    using callback_token = notifier_type::callback_token;
    using callback_proto = notifier_type::callback_proto;

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
    [[nodiscard]] virtual generator<audio_device &> devices() noexcept = 0;

    /** Subscribe a function to be called when the device list changes.
     *
     * @return A callback token, a RAII object which when destroyed removes the subscription.
     */
    callback_token subscribe(forward_of<callback_proto> auto&& func, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return _notifier.subscribe(hi_forward(func), flags);
    }

    auto operator co_await() noexcept
    {
        return _notifier.operator co_await();
    }

protected:
    notifier_type _notifier;
};

} // namespace hi::inline v1