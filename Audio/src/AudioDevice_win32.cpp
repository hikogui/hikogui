// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Audio/AudioDevice_win32.hpp"
#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/exceptions.hpp"
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

    hresult_assert_or_throw(propertyStore_->GetValue(PKEY_Device_FriendlyName, &textProperty));
    auto textWString = std::wstring_view(textProperty.pwszVal);
    auto textString = to_string(textWString);

    PropVariantClear(&textProperty);
    return textString;
}

AudioDevice_win32::AudioDevice_win32(void *device) :
    AudioDevice(), device(device)
{
    tt_assert(device != nullptr);

    id = getIdFromDevice(device);
    
    auto device_ = static_cast<IMMDevice *>(device);
    
    hresult_assert_or_throw(device_->OpenPropertyStore(STGM_READ, reinterpret_cast<IPropertyStore **>(&propertyStore)));
}

AudioDevice_win32::~AudioDevice_win32()
{
    auto propertyStore_ = static_cast<IPropertyStore *>(propertyStore);
    tt_assert(propertyStore_ != nullptr);
    propertyStore_->Release();

    auto device_ = static_cast<IMMDevice *>(device);
    tt_assert(device_ != nullptr);
    device_->Release();
}

std::string AudioDevice_win32::name() const noexcept
{
    return getStringProperty(propertyStore, PKEY_Device_FriendlyName);
}

std::string AudioDevice_win32::deviceName() const noexcept
{
    return getStringProperty(propertyStore, PKEY_DeviceInterface_FriendlyName);
}

std::string AudioDevice_win32::endPointName() const noexcept
{
    return getStringProperty(propertyStore, PKEY_Device_DeviceDesc);
}

AudioDevice_state AudioDevice_win32::state() const noexcept
{
    auto device_ = static_cast<IMMDevice *>(device);

    DWORD state;
    hresult_assert_or_throw(device_->GetState(&state));

    switch (state) {
    case DEVICE_STATE_ACTIVE:
        return AudioDevice_state::Active;
    case DEVICE_STATE_DISABLED:
        return AudioDevice_state::Disabled;
    case DEVICE_STATE_NOTPRESENT:
        return AudioDevice_state::NotPresent;
    case DEVICE_STATE_UNPLUGGED:
        return AudioDevice_state::Unplugged;
    default:
        tt_no_default;
    }
}

std::string AudioDevice_win32::getIdFromDevice(void *device) noexcept
{
    auto device_ = static_cast<IMMDevice *>(device);

    // Get the cross-reboot-unique-id-string of the device.
    LPWSTR id_wcharstr;
    tt_assert(device_ != nullptr);
    hresult_assert_or_throw(device_->GetId(&id_wcharstr));

    ttlet id_wstring = std::wstring_view(id_wcharstr);
    auto id = to_string(id_wstring);
    CoTaskMemFree(id_wcharstr);
    return id;
}

}
