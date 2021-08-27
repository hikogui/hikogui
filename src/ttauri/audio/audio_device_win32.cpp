// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_device_win32.hpp"
#include "audio_sample_format.hpp"
#include "audio_stream_format.hpp"
#include "audio_stream_format_win32.hpp"
#include "speaker_mapping.hpp"
#include "speaker_mapping_win32.hpp"
#include "../logger.hpp"
#include "../strings.hpp"
#include "../exception.hpp"
#include "../cast.hpp"
#include <Windows.h>
#include <mmreg.h>
#include <propsys.h>
#include <initguid.h>
#include <functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audioclient.h>
#include <bit>

namespace tt {

[[nodiscard]] static WAVEFORMATEXTENSIBLE make_wave_format(
    audio_sample_format format,
    uint16_t num_channels,
    speaker_mapping speaker_mapping,
    uint32_t sample_rate) noexcept
{
    tt_axiom(std::popcount(static_cast<size_t>(speaker_mapping)) <= num_channels);

    bool extended = false;

    // legacy format can only handle mono or stereo.
    extended |= num_channels > 2;

    // Legacy format can only handle bits equal to container size.
    extended |= (format.num_bytes * 8) != (format.num_guard_bits + format.num_bits + 1);

    // Legacy format can only handle direct channel map. This allows you to select legacy
    // mono and stereo for old device drivers.
    extended |= speaker_mapping != speaker_mapping::direct;

    // Legacy format can only be PCM-8, PCM-16 or PCM-float-32.
    if (format.is_float) {
        extended |= format.num_bytes == 4;
    } else {
        extended |= format.num_bytes <= 2;
    }

    WAVEFORMATEXTENSIBLE r;
    r.Format.wFormatTag = extended ? WAVE_FORMAT_EXTENSIBLE : format.is_float ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
    r.Format.nChannels = num_channels;
    r.Format.nSamplesPerSec = sample_rate;
    r.Format.nAvgBytesPerSec = narrow_cast<DWORD>(sample_rate * num_channels * format.num_bytes);
    r.Format.nBlockAlign = narrow_cast<WORD>(num_channels * format.num_bytes);
    r.Format.wBitsPerSample = narrow_cast<WORD>(format.num_bytes * 8);
    r.Format.cbSize = extended ? (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)) : 0;
    r.Samples.wValidBitsPerSample = narrow_cast<WORD>(format.num_guard_bits + format.num_bits + 1);
    r.dwChannelMask = speaker_mapping_to_win32(speaker_mapping);
    r.SubFormat = format.is_float ? KSDATAFORMAT_SUBTYPE_IEEE_FLOAT : KSDATAFORMAT_SUBTYPE_PCM;
    return r;
}

[[nodiscard]] static WAVEFORMATEXTENSIBLE
make_wave_format(audio_sample_format format, uint16_t num_channels, uint32_t sample_rate) noexcept
{
    return make_wave_format(format, num_channels, speaker_mapping::direct, sample_rate);
}

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

[[nodiscard]] audio_device_id audio_device_win32::get_id(IMMDevice *device) noexcept
{
    // Get the cross-reboot-unique-id-string of the device.
    LPWSTR device_id;
    tt_hresult_check(device->GetId(&device_id));
    auto device_id_ = audio_device_id{audio_device_id::win32, device_id};

    CoTaskMemFree(device_id);
    return device_id_;
}

audio_device_win32::audio_device_win32(IMMDevice *device) : audio_device(), _device(device)
{
    tt_assert(_device != nullptr);
    id = get_id(_device);
    tt_hresult_check(_device->QueryInterface(&_end_point));
    tt_hresult_check(_device->OpenPropertyStore(STGM_READ, &_property_store));

    if (FAILED(_device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, reinterpret_cast<void **>(&_audio_client)))) {
        tt_log_warning("Audio device {} does not have IAudioClient interface", name());
        _audio_client = nullptr;
    }

    // By setting exclusivity to false at the start the audio stream format is initialized properly.
    set_exclusive(false);
}

audio_device_win32::~audio_device_win32()
{
    _property_store->Release();
    _end_point->Release();
    _device->Release();

    if (_audio_client != nullptr) {
        _audio_client->Release();
    }
}

[[nodiscard]] bool audio_device_win32::supports_format(audio_stream_format const &format) const noexcept
{
    auto format_ = audio_stream_format_to_win32(format);

    auto r = _audio_client->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, reinterpret_cast<WAVEFORMATEX *>(&format_), NULL);
    if (r == S_OK) {
        return true;
    } else if (r == S_FALSE or r == AUDCLNT_E_UNSUPPORTED_FORMAT) {
        return false;
    } else {
        tt_log_error("Failed to check format. {}", get_last_error_message());
        return false;
    }
}

[[nodiscard]] bool audio_device_win32::exclusive() const noexcept
{
    return _exclusive;
}

[[nodiscard]] audio_stream_format
audio_device_win32::find_exclusive_stream_format(double sample_rate, tt::speaker_mapping speaker_mapping) noexcept
{
    return {};
}

void audio_device_win32::set_exclusive(bool exclusive) noexcept
{
    if (exclusive) {
        _current_stream_format = find_exclusive_stream_format(_sample_rate, _speaker_mapping);
    } else {
        _current_stream_format = shared_stream_format();
    }

    _exclusive = exclusive;
}

[[nodiscard]] double audio_device_win32::sample_rate() const noexcept
{
    return _sample_rate;
}

void audio_device_win32::set_sample_rate(double sample_rate) noexcept
{
    _sample_rate = sample_rate;
}

[[nodiscard]] tt::speaker_mapping audio_device_win32::input_speaker_mapping() const noexcept
{
    switch (_direction) {
    case audio_direction::input: [[fallthrough]];
    case audio_direction::bidirectional: return _speaker_mapping;
    case audio_direction::output: return tt::speaker_mapping::direct;
    default: tt_no_default();
    }
}

void audio_device_win32::set_input_speaker_mapping(tt::speaker_mapping speaker_mapping) noexcept
{
    switch (_direction) {
    case audio_direction::input: [[fallthrough]];
    case audio_direction::bidirectional: _speaker_mapping = speaker_mapping; break;
    case audio_direction::output: break;
    default: tt_no_default();
    }
}

[[nodiscard]] std::vector<tt::speaker_mapping> audio_device_win32::available_input_speaker_mappings() const noexcept
{
    return {};
}

[[nodiscard]] tt::speaker_mapping audio_device_win32::output_speaker_mapping() const noexcept
{
    switch (_direction) {
    case audio_direction::output: [[fallthrough]];
    case audio_direction::bidirectional: return _speaker_mapping;
    case audio_direction::input: return tt::speaker_mapping::direct;
    default: tt_no_default();
    }
}

void audio_device_win32::set_output_speaker_mapping(tt::speaker_mapping speaker_mapping) noexcept
{
    switch (_direction) {
    case audio_direction::output: [[fallthrough]];
    case audio_direction::bidirectional: _speaker_mapping = speaker_mapping; break;
    case audio_direction::input: break;
    default: tt_no_default();
    }
}

[[nodiscard]] std::vector<tt::speaker_mapping> audio_device_win32::available_output_speaker_mappings() const noexcept
{
    return {};
}

/*[[nodiscard]] std::optional<audio_stream_format>
audio_device_win32::best_format(double sample_rate, tt::speaker_mapping speaker_mapping) const noexcept
{
    constexpr auto sample_formats = std::array{
        audio_sample_format::float32(),
        audio_sample_format::fix8_23(),
        audio_sample_format::int24(),
        audio_sample_format::int20(),
        audio_sample_format::int16()};

    for (ttlet &sample_format : sample_formats) {
        ttlet format = audio_stream_format{sample_format, sample_rate, speaker_mapping};
        if (supports_format(format)) {
            return format;
        }
    }
    return {};
}

[[nodiscard]] std::optional<audio_stream_format> audio_device_win32::best_format(double sample_rate, int num_channels) const
noexcept
{
    constexpr auto sample_formats = std::array{
        audio_sample_format::float32(),
        audio_sample_format::fix8_23(),
        audio_sample_format::int24(),
        audio_sample_format::int20(),
        audio_sample_format::int16()};

    for (ttlet &sample_format : sample_formats) {
        ttlet format = audio_stream_format{sample_format, sample_rate, num_channels};
        if (supports_format(format)) {
            return format;
        }
    }
    return {};
}

[[nodiscard]] std::vector<audio_stream_format> audio_device_win32::enumerate_formats(double sample_rate) const noexcept
{
    constexpr max_num_channels = 128;

    auto r = std::vector<audio_stream_format>{};
    r.reserve(std::size(speaker_mappings) + (max_num_channels / 2) + 2);

    for (ttlet &info: speaker_mappings) {
        if (auto f = best_format(sample_rate, info.mapping)) {
            r.push_back(*std::move(f));
        }
    }
    for (auto num_channels = 1; num_channels != 4; ++num_channels) {
        if (auto f = best_format(sample_rate, num_channels)) {
            r.push_back(*std::move(f));
        }
    }
    for (auto num_channels = 4; num_channels <= max_num_channels; num_channels += 2) {
        if (auto f = best_format(sample_rate, num_channels)) {
            r.push_back(*std::move(f));
        }
    }

    return r;
}*/

std::string audio_device_win32::name() const noexcept
{
    try {
        return get_property<std::string>(_property_store, PKEY_Device_FriendlyName);
    } catch (io_error const &) {
        return "<unknown name>";
    }
}

tt::label audio_device_win32::label() const noexcept
{
    return {elusive_icon::Speaker, l10n("{}", name())};
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
        return "<unknown device name>";
    }
}

std::string audio_device_win32::end_point_name() const noexcept
{
    try {
        return get_property<std::string>(_property_store, PKEY_Device_DeviceDesc);
    } catch (io_error const &) {
        return "<unknown end point name>";
    }
}

/** Get the shared stream format for the device.
 */
[[nodiscard]] audio_stream_format audio_device_win32::shared_stream_format() const
{
    if (not _audio_client) {
        return {};
    }

    WAVEFORMATEX *ex;
    tt_hresult_check(_audio_client->GetMixFormat(&ex));
    tt_axiom(ex);
    ttlet r = audio_stream_format_from_win32(*ex);
    CoTaskMemFree(ex);
    return r;
}

} // namespace tt
