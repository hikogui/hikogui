// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_device_win32.hpp"
#include "../logger.hpp"
#include "../strings.hpp"
#include "../exception.hpp"
#include <Windows.h>
#include <propsys.h>
#include <functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>

namespace tt {

static std::string getStringProperty(void *propertyStore, REFPROPERTYKEY key)
{
    auto propertyStore_ = static_cast<IPropertyStore *>(propertyStore);
    tt_assert(propertyStore_ != nullptr);

    PROPVARIANT textProperty;
    PropVariantInit(&textProperty);

    tt_hresult_check(propertyStore_->GetValue(key, &textProperty));
    auto textWString = std::wstring_view(textProperty.pwszVal);
    auto textString = to_string(textWString);

    PropVariantClear(&textProperty);
    return textString;
}

audio_device_win32::audio_device_win32(IMMDevice *device) : audio_device(), _device(device)
{
    tt_assert(_device != nullptr);
    tt_hresult_check(_device->QueryInterface(&_endpoint));
    tt_hresult_check(_device->OpenPropertyStore(STGM_READ, &_property_store));
}

audio_device_win32::~audio_device_win32()
{
    _property_store->Release();
    _endpoint->Release();
    _device->Release();
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
        return getStringProperty(_property_store, PKEY_Device_FriendlyName);
    } catch (io_error const &) {
        return "<unknown name>"s;
    }
}

tt::label audio_device_win32::label() const noexcept
{
    return {elusive_icon::Speaker, l10n("{}"), name()};
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

audio_device_flow_direction audio_device_win32::direction() const noexcept
{
    EDataFlow data_flow;
    tt_hresult_check(_endpoint->GetDataFlow(&data_flow));

    switch (data_flow) {
    case eRender: return audio_device_flow_direction::output;
    case eCapture: return audio_device_flow_direction::input;
    case eAll: return audio_device_flow_direction::bidirectional;
    default: tt_no_default();
    }
}

std::string audio_device_win32::device_name() const noexcept
{
    try {
        return getStringProperty(_property_store, PKEY_DeviceInterface_FriendlyName);
    } catch (io_error const &) {
        return "<unknown device name>"s;
    }
}

std::string audio_device_win32::end_point_name() const noexcept
{
    try {
        return getStringProperty(_property_store, PKEY_Device_DeviceDesc);
    } catch (io_error const &) {
        return "<unknown end point name>"s;
    }
}

} // namespace tt
