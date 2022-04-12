// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_system.hpp"
#include "audio_system_delegate.hpp"
#include "audio_device_id.hpp"
#include "../wfree_fifo.hpp"
#include <memory>

struct IMMDeviceEnumerator;

namespace tt::inline v1 {

class audio_system_win32_notification_client;

class audio_system_win32;

struct audio_system_win32_event {
    virtual void handle_event(audio_system_win32 *self) noexcept = 0;
};

class audio_system_win32 : public audio_system {
public:
    using super = audio_system;

    audio_system_win32(std::weak_ptr<audio_system_delegate> delegate);
    ~audio_system_win32();

    void init() noexcept override;

    [[nodiscard]] std::vector<audio_device *> devices() noexcept override
    {
        auto r = std::vector<audio_device *>{};
        r.reserve(size(_devices));
        for (ttlet &device : _devices) {
            r.push_back(device.get());
        }
        return r;
    }

    void update_device_list() noexcept;

private:
    /** The devices that are part of the audio system.
     *
     * Due to complicated threading and callback function interactions
     * audio devices are not destroyed until application shutdown.
     *
     * The audio system is the only owner of audio devices, however
     * audio devices need to be allocated on locked memory, and
     * unique_ptr does not support allocators.
     */
    std::vector<std::shared_ptr<audio_device>> _devices;

    IMMDeviceEnumerator *_device_enumerator;
    audio_system_win32_notification_client *_notification_client;

    void default_device_changed(tt::audio_device_id const &device_id) noexcept;
    void device_added(tt::audio_device_id const &device_id) noexcept;
    void device_removed(tt::audio_device_id const &device_id) noexcept;
    void device_state_changed(tt::audio_device_id const &device_id) noexcept;
    void device_property_value_changed(tt::audio_device_id const &device_id) noexcept;

    friend audio_system_win32_notification_client;
};

} // namespace tt::inline v1