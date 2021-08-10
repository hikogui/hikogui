// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_system_delegate.hpp"
#include "audio_device.hpp"
#include "../unfair_recursive_mutex.hpp"
#include "../weak_or_unique_ptr.hpp"
#include <vector>
#include <memory>

namespace tt {
class event_queue;

/*! An system of audio devices.
 * Systems are for example: Window Audio Session API (WASAPI), ASIO, Apple CoreAudio
 */
class audio_system {
public:
    [[nodiscard]] static std::unique_ptr<audio_system>
    make_unique(tt::event_queue const &event_queue, std::weak_ptr<audio_system_delegate> delegate) noexcept;

    audio_system(tt::event_queue const &event_queue, std::weak_ptr<audio_system_delegate> delegate);
    virtual ~audio_system();
    audio_system(audio_system const &) = delete;
    audio_system(audio_system &&) = delete;
    audio_system &operator=(audio_system const &) = delete;
    audio_system &operator=(audio_system &&) = delete;

    virtual void init() noexcept;
    virtual void deinit() noexcept;

    /** The devices that are part of the audio system.
     *
     * Due to complicated threading and callback function interactions
     * audio devices are not destroyed until application shutdown.
     */
    [[nodiscard]] virtual std::vector<audio_device *> devices() noexcept = 0;

protected:
    tt::event_queue const &_event_queue;

    std::weak_ptr<audio_system_delegate> _delegate;
};

} // namespace tt