// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_device_win32.hpp"
#include "../logger.hpp"
#include "../strings.hpp"
#include "../exception.hpp"
#include <Windows.h>
#include <propsys.h>
#include <initguid.h>
#include <functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <bit>

namespace tt {

template<typename T>
[[nodiscard]] T get_property(IPropertyStore *property_store, REFPROPERTYKEY key)
{
    tt_not_implemented();
}

template<>
[[nodiscard]] std::string get_property(IPropertyStore *property_store, REFPROPERTYKEY key)
{
    tt_assert(property_store != nullptr);

    PROPVARIANT property_value;
    PropVariantInit(&property_value);

    tt_hresult_check(property_store->GetValue(key, &property_value));
    if (property_value.vt == VT_LPWSTR) {
        auto value_wstring = std::wstring_view(property_value.pwszVal);
        auto value_string = to_string(value_wstring);
        PropVariantClear(&property_value);
        return value_string;

    } else {
        PropVariantClear(&property_value);
        throw io_error("Unexpected property value type {}.", static_cast<int>(property_value.vt));
    }
}

template<>
[[nodiscard]] uint32_t get_property(IPropertyStore *property_store, REFPROPERTYKEY key)
{
    tt_assert(property_store != nullptr);

    PROPVARIANT property_value;
    PropVariantInit(&property_value);

    tt_hresult_check(property_store->GetValue(key, &property_value));
    if (property_value.vt == VT_UI4) {
        auto value = narrow_cast<uint32_t>(property_value.ulVal);
        PropVariantClear(&property_value);
        return value;

    } else {
        PropVariantClear(&property_value);
        throw io_error("Unexpected property value type {}.", static_cast<int>(property_value.vt));
    }
}

audio_device_win32::audio_device_win32(IMMDevice *device) : audio_device(), _device(device)
{
    tt_assert(_device != nullptr);
    tt_hresult_check(_device->QueryInterface(&_end_point));
    tt_hresult_check(_device->OpenPropertyStore(STGM_READ, &_property_store));

    if (FAILED(
            _device->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, reinterpret_cast<void **>(&_end_point_volume)))) {
        tt_log_warning("Audio device {} does not have IAudioEndPointVolume interface", name());
        _end_point_volume = nullptr;
    }

    if (FAILED(_device->Activate(
            __uuidof(IAudioMeterInformation), CLSCTX_ALL, NULL, reinterpret_cast<void **>(&_audio_meter_information)))) {
        tt_log_warning("Audio device {} does not have IAudioMeterInformation interface", name());
        _audio_meter_information = nullptr;
    }
}

audio_device_win32::~audio_device_win32()
{
    _property_store->Release();
    _end_point->Release();
    _device->Release();
    if (_end_point_volume != nullptr) {
        _end_point_volume->Release();
    }
    if (_audio_meter_information != nullptr) {
        _audio_meter_information->Release();
    }
}

std::string audio_device_win32::get_id_from_device(IMMDevice *device) noexcept
{
    // Get the cross-reboot-unique-id-string of the device.
    LPWSTR id_wcharstr;
    tt_hresult_check(device->GetId(&id_wcharstr));

    ttlet id_wstring = std::wstring_view(id_wcharstr);
    auto id = to_string(id_wstring);
    CoTaskMemFree(id_wcharstr);
    return "win32:"s + id;
}

std::string audio_device_win32::id() const noexcept
{
    return get_id_from_device(_device);
}

std::string audio_device_win32::name() const noexcept
{
    try {
        return get_property<std::string>(_property_store, PKEY_Device_FriendlyName);
    } catch (io_error const &) {
        return "<unknown name>"s;
    }
}

tt::label audio_device_win32::label() const noexcept
{
    return {elusive_icon::Speaker, l10n{"{}", name()}};
}

audio_device_state audio_device_win32::state() const noexcept
{
    DWORD state;
    tt_hresult_check(_device->GetState(&state));

    switch (state) {
    case DEVICE_STATE_ACTIVE: return audio_device_state::active;
    case DEVICE_STATE_DISABLED: return audio_device_state::disabled;
    case DEVICE_STATE_NOTPRESENT: return audio_device_state::not_present;
    case DEVICE_STATE_UNPLUGGED: return audio_device_state::unplugged;
    default: tt_no_default();
    }
}

audio_direction audio_device_win32::direction() const noexcept
{
    EDataFlow data_flow;
    tt_hresult_check(_end_point->GetDataFlow(&data_flow));

    switch (data_flow) {
    case eRender: return audio_direction::output;
    case eCapture: return audio_direction::input;
    case eAll: return audio_direction::bidirectional;
    default: tt_no_default();
    }
}

std::string audio_device_win32::device_name() const noexcept
{
    try {
        return get_property<std::string>(_property_store, PKEY_DeviceInterface_FriendlyName);
    } catch (io_error const &) {
        return "<unknown device name>"s;
    }
}

std::string audio_device_win32::end_point_name() const noexcept
{
    try {
        return get_property<std::string>(_property_store, PKEY_Device_DeviceDesc);
    } catch (io_error const &) {
        return "<unknown end point name>"s;
    }
}

[[nodiscard]] size_t audio_device_win32::full_num_channels() const noexcept
{
    auto r = 0_uz;

    if (_end_point_volume != nullptr) {
        try {
            UINT channel_count;
            tt_hresult_check(_end_point_volume->GetChannelCount(&channel_count));
            r = std::max(r, narrow_cast<size_t>(channel_count));
        } catch (io_error const &) {
        }
    }

    if (_audio_meter_information != nullptr) {
        try {
            UINT channel_count;
            tt_hresult_check(_audio_meter_information->GetMeteringChannelCount(&channel_count));
            r = std::max(r, narrow_cast<size_t>(channel_count));
        } catch (io_error const &) {
        }
    }

    [[maybe_unused]] auto mapping = full_channel_mapping();
    return r;
}

[[nodiscard]] audio_channel_mapping audio_device_win32::full_channel_mapping() const noexcept
{
    try {
        ttlet win32_spaker_mapping = get_property<uint32_t>(_property_store, PKEY_AudioEndpoint_PhysicalSpeakers);
        return audio_channel_mapping_from_win32(win32_spaker_mapping);
    } catch (io_error const &) {
        return audio_channel_mapping::front_left | audio_channel_mapping::front_right;
    }
}

} // namespace tt
