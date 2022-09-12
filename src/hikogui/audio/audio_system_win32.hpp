// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_system.hpp"
#include "../wfree_fifo.hpp"
#include <memory>

struct IMMDeviceEnumerator;

namespace hi::inline v1 {
class audio_system_win32_notification_client;
class audio_system_win32;

struct audio_system_win32_event {
    virtual void handle_event(audio_system_win32 *self) noexcept = 0;
};

class audio_system_win32 : public audio_system {
public:
    using super = audio_system;

    audio_system_win32();
    virtual ~audio_system_win32();

    [[nodiscard]] generator<audio_device *> devices() noexcept override
    {
        for (hilet &device : _devices) {
            co_yield device.get();
        }
    }

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
    std::unique_ptr<audio_system_win32_notification_client> _notification_client;

    /** Before clients are notified, this function is called to update the device list.
     */
    void update_device_list() noexcept;

    friend class audio_system_win32_notification_client;
};

} // namespace hi::inline v1