// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Audio/AudioDevice_win32.hpp"
#include "TTauri/Diagnostic/logger.hpp"
#include "TTauri/Required/strings.hpp"
#include <Windows.h>
#include <propsys.h>
#include <functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>

namespace TTauri::Audio {

static std::string getStringProperty(void *propertyStore, REFPROPERTYKEY key)
{
    auto propertyStore_ = static_cast<IPropertyStore *>(propertyStore);
    required_assert(propertyStore_ != nullptr);

    PROPVARIANT textProperty;
    PropVariantInit(&textProperty);

    hresult_assert(propertyStore_->GetValue(PKEY_Device_FriendlyName, &textProperty));
    auto textWString = std::wstring_view(textProperty.pwszVal);
    auto textString = translateString<std::string>(textWString);

    PropVariantClear(&textProperty);
    return textString;
}

AudioDevice_win32::AudioDevice_win32(void *device) :
    AudioDevice(), device(device)
{
    required_assert(device != nullptr);

    id = getIdFromDevice(device);
    
    auto device_ = static_cast<IMMDevice *>(device);
    
    hresult_assert(device_->OpenPropertyStore(STGM_READ, reinterpret_cast<IPropertyStore **>(&propertyStore)));
}

AudioDevice_win32::~AudioDevice_win32()
{
    auto propertyStore_ = static_cast<IPropertyStore *>(propertyStore);
    required_assert(propertyStore_ != nullptr);
    propertyStore_->Release();

    auto device_ = static_cast<IMMDevice *>(device);
    required_assert(device_ != nullptr);
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

std::string AudioDevice_win32::getIdFromDevice(void *device) noexcept
{
    auto device_ = static_cast<IMMDevice *>(device);

    // Get the cross-reboot-unique-id-string of the device.
    LPWSTR id_wcharstr;
    required_assert(device_ != nullptr);
    hresult_assert(device_->GetId(&id_wcharstr));

    let id_wstring = std::wstring_view(id_wcharstr);
    auto id = translateString<std::string>(id_wstring);
    CoTaskMemFree(id_wcharstr);
    return id;
}

}
