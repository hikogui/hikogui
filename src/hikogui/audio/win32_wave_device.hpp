// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_direction.hpp"
#include "../generator.hpp"
#include <string>
#include <Windows.h>

namespace hi::inline v1 {

class win32_wave_device {
public:
    UINT id;
    audio_direction direction;

    /** The end-point-id matching end-point ids of the modern Core Audio MMDevice API.
     */
    [[nodiscard]] std::string end_point_id() const;

    /** Open the audio device.
     *
     * @return A file handle to the audio device.
     */
    [[nodiscard]] HANDLE open_device_interface() const;

    [[nodiscard]] static UINT num_devices(audio_direction direction) noexcept;

    [[nodiscard]] static generator<win32_wave_device> enumerate(audio_direction direction) noexcept;

    [[nodiscard]] static win32_wave_device find_matching_end_point(audio_direction direction, std::string end_point_id) noexcept;

private:
    [[nodiscard]] std::wstring message_wstring(UINT message_id, UINT size_message_id) const;
    [[nodiscard]] std::string message_string(UINT message_id, UINT size_message_id) const;
};

} // namespace hi::inline v1
