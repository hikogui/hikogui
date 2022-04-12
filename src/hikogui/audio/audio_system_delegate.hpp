// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_block.hpp"

namespace hi::inline v1 {
class audio_system;

class audio_system_delegate {
public:
    virtual ~audio_system_delegate() = default;
    virtual void init(audio_system &sender) noexcept {}
    virtual void deinit(audio_system &sender) noexcept {}

    /*! Called when the device list has changed.
     * This can happen when external devices are connected or disconnected.
     */
    virtual void audio_device_list_changed(audio_system &self) {}
};

} // namespace hi::inline v1