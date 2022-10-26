// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../win32_headers.hpp"

#include "win32_wave_device.hpp"
#include "../utility.hpp"
#include "../exception.hpp"

namespace hi::inline v1 {

[[nodiscard]] UINT win32_wave_device::num_devices(audio_direction direction) noexcept
{
    return direction == audio_direction::input ? waveInGetNumDevs() : waveOutGetNumDevs();
}

[[nodiscard]] generator<win32_wave_device> win32_wave_device::enumerate(audio_direction direction) noexcept
{
    auto num = num_devices(direction);

    for (UINT id = 0; id != num; ++id) {
        co_yield win32_wave_device{id, direction};
    }
}

[[nodiscard]] win32_wave_device
win32_wave_device::find_matching_end_point(audio_direction direction, std::string end_point_id)
{
    for (auto wave_api : enumerate(direction)) {
        if (wave_api.end_point_id() == end_point_id) {
            return wave_api;
        }
    }
    throw io_error(std::format("Could not find matching wave device for end-point-id {}", end_point_id));
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
[[nodiscard]] win32_device_interface win32_wave_device::open_device_interface() const
{
    auto device_name = message_string(DRV_QUERYDEVICEINTERFACE, DRV_QUERYDEVICEINTERFACESIZE);
    return win32_device_interface{device_name};
}

[[nodiscard]] std::wstring win32_wave_device::message_wstring(UINT message_id, UINT size_message_id) const
{
    DWORD size = 0;
    {
        hilet result = _direction == audio_direction::input ?
            waveInMessage((HWAVEIN)IntToPtr(_id), size_message_id, std::bit_cast<DWORD_PTR>(&size), NULL) :
            waveOutMessage((HWAVEOUT)IntToPtr(_id), size_message_id, std::bit_cast<DWORD_PTR>(&size), NULL);

        if (result != MMSYSERR_NOERROR) {
            throw io_error(std::format(
                "Could not get win32_wave_api wstring-message {} for wave-device-id:{}:{}", size_message_id, _direction, _id));
        }
    }

    // The length is in bytes, including the terminating wchar_t{}.
    hi_assert(size > 0 and size % sizeof(wchar_t) == 0);
    auto str = std::wstring(size / sizeof(wchar_t) - 1, wchar_t{});
    {
        hilet result = _direction == audio_direction::input ?
            waveInMessage((HWAVEIN)IntToPtr(_id), message_id, std::bit_cast<DWORD_PTR>(str.data()), size) :
            waveOutMessage((HWAVEOUT)IntToPtr(_id), message_id, std::bit_cast<DWORD_PTR>(str.data()), size);

        if (result != MMSYSERR_NOERROR) {
            throw io_error(std::format(
                "Could not get win32_wave_api wstring-message {} for wave-device-id:{}:{}", message_id, _direction, _id));
        }
    }

    return str;
}

[[nodiscard]] std::string win32_wave_device::message_string(UINT message_id, UINT size_message_id) const
{
    return to_string(message_wstring(message_id, size_message_id));
}

} // namespace hi::inline v1
