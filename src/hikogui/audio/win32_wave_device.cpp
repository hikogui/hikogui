// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "win32_wave_device.hpp"
#include "../required.hpp"
#include "../exception.hpp"
#include <Windows.h>
#include <mmsystem.h>
#include <mmeapi.h>
#include <mmddk.h>

namespace hi::inline v1 {

[[nodiscard]] UINT win32_wave_device::num_devices(audio_direction direction) noexcept
{
    return direction == audio_direction::input ? waveInGetNumDevs() : waveOutGetNumDevs();
}

[[nodiscard]] generator<win32_wave_device> win32_wave_device::enumerate(audio_direction direction) noexcept
{
    auto num_devices = num_devices(direction);

    for (UINT id = 0; id != num_devices; ++id) {
        co_yield win32_wave_api{id, direction};
    }
}

[[nodiscard]] win32_wave_device
win32_wave_device::find_matching_end_point(audio_direction direction, std::string end_point_id) noexcept
{
    for (auto wave_api = enumerate(direction)) {
        if (wave_api.end_point_id() == end_point_id) {
            return wave_api;
        }
    }
    throw io_error("Could not find matching wave device for end-point-id {}", end_point_id);
}

/** The end-point-id matching end-point ids of the modern Core Audio MMDevice API.
 */
[[nodiscard]] std::string win32_wave_device::end_point_id() const
{
    return message_string(DRV_QUERYFUNCTIONINSTANCEID, DRV_QUERYFUNCTIONINSTANCEIDSIZE);
}

/** Open the audio device.
 *
 * @return A file handle to the audio device.
 */
[[nodiscard]] HANDLE win32_wave_device::open_device_interface() const
{
    auto device_path = message_wstring(DRV_QUERYDEVICEINTERFACE, DRV_QUERYDEVICEINTERFACESIZE);

    HANDLE result = CreateFileW(device_path.c_str(), FILE_SHARE_READ | FILE_SHARE_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        throw io_error("Could not open device interface for wave device {}:{}: {}", direction, id, get_last_error_message());
    }
    return handle;
}

[[nodiscard]] std::wstring win32_wave_device::message_wstring(UINT message_id, UINT size_message_id) const
{
    DWORD size = 0;
    {
        hilet result = direction == audio_direction::input ?
            waveInMessage((HWAVEIN)IntToPtr(i), size_message_id, std::bit_cast<DWORD_PTR>(&size), NULL) :
            waveOutMessage((HWAVEOUT)IntToPtr(i), size_message_id, std::bit_cast<DWORD_PTR>(&size), NULL);

        if (result != MMSYSERR_NOERROR) {
            throw io_error(std::format(
                "Could not get win32_wave_api wstring-message {} for wave-device-id:{}:{}", size_message_id, direction, id));
        }
    }

    // The length is in bytes, including the terminating wchar_t{}.
    hi_assert(length > 0 and length % sizeof(wchar_t) == 0);
    auto str = std::wstring(length / sizeof(wchar_t) - 1, wchar_t{});
    {
        hilet result = direction == audio_direction::input ?
            waveInMessage((HWAVEIN)IntToPtr(i), message_id, std::bit_cast<DWORD_PTR>(str.data()), size) :
            waveOutMessage((HWAVEOUT)IntToPtr(i), message_id, std::bit_cast<DWORD_PTR>(str.data()), size);

        if (result != MMSYSERR_NOERROR) {
            throw io_error(std::format(
                "Could not get win32_wave_api wstring-message {} for wave-device-id:{}:{}", message_id, direction, id));
        }
    }

    return str;
}

[[nodiscard]] std::string win32_wave_device::message_string(UINT message_id, UINT size_message_id) const
{
    return to_string(message_wstring(message_id, size_message_id);
}

} // namespace hi::inline v1
