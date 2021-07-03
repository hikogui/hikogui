// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_system.hpp"
#include "audio_system_delegate.hpp"
#include <memory>

struct IMMDeviceEnumerator;

namespace tt {

class audio_system_win32_notification_client;

class audio_system_win32: public audio_system {
public:
    using super = audio_system;

    audio_system_win32(weak_or_unique_ptr<audio_system_delegate> delegate);
    ~audio_system_win32();

    void init() noexcept override;

    [[nodiscard]] std::vector<std::shared_ptr<audio_device>> devices() noexcept override
    {
        ttlet lock = std::scoped_lock(audio_system::mutex);
        return _devices;
    }

    void update_device_list() noexcept;

private:
    std::vector<std::shared_ptr<audio_device>> _devices;
    IMMDeviceEnumerator *_device_enumerator;
    audio_system_win32_notification_client *_notification_client;

    void default_device_changed() noexcept;
    void device_added() noexcept;
    void device_removed(std::string device_id) noexcept;
    void device_state_changed(std::string device_id) noexcept;
    void device_property_value_changed(std::string device_id) noexcept;

    friend audio_system_win32_notification_client;
};

}