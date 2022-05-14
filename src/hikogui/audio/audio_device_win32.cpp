// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_device_win32.hpp"
#include "audio_sample_format.hpp"
#include "audio_stream_format.hpp"
#include "audio_stream_format_win32.hpp"
#include "speaker_mapping.hpp"
#include "speaker_mapping_win32.hpp"
#include "win32_wave_device.hpp"
#include "../log.hpp"
#include "../strings.hpp"
#include "../exception.hpp"
#include "../cast.hpp"
#include <Windows.h>

#include <mmsystem.h>
#include <mmreg.h>
#include <mmeapi.h>
#include <mmddk.h>
#include <propsys.h>
#include <initguid.h>
#include <functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audioclient.h>
#include <bit>

namespace hi::inline v1 {

[[nodiscard]] static WAVEFORMATEXTENSIBLE make_wave_format(
    audio_sample_format format,
    uint16_t num_channels,
    speaker_mapping speaker_mapping,
    uint32_t sample_rate) noexcept
{
    hi_axiom(std::popcount(static_cast<std::size_t>(speaker_mapping)) <= num_channels);

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
    hi_not_implemented();
}

template<>
[[nodiscard]] std::string get_property(IPropertyStore *property_store, REFPROPERTYKEY key)
{
    hi_assert(property_store != nullptr);

    PROPVARIANT property_value;
    PropVariantInit(&property_value);

    hi_hresult_check(property_store->GetValue(key, &property_value));
    if (property_value.vt == VT_LPWSTR) {
        auto value_wstring = std::wstring_view(property_value.pwszVal);
        auto value_string = to_string(value_wstring);
        PropVariantClear(&property_value);
        return value_string;

    } else {
        PropVariantClear(&property_value);
        throw io_error(std::format("Unexpected property value type {}.", static_cast<int>(property_value.vt)));
    }
}

template<>
[[nodiscard]] uint32_t get_property(IPropertyStore *property_store, REFPROPERTYKEY key)
{
    hi_assert(property_store != nullptr);

    PROPVARIANT property_value;
    PropVariantInit(&property_value);

    hi_hresult_check(property_store->GetValue(key, &property_value));
    if (property_value.vt == VT_UI4) {
        auto value = narrow_cast<uint32_t>(property_value.ulVal);
        PropVariantClear(&property_value);
        return value;

    } else {
        PropVariantClear(&property_value);
        throw io_error(std::format("Unexpected property value type {}.", static_cast<int>(property_value.vt)));
    }
}

[[nodiscard]] std::string audio_device_win32::get_device_id(IMMDevice *device)
{
    hi_assert(device);

    // Get the cross-reboot-unique-id-string of the device.
    LPWSTR device_id;
    hi_hresult_check(device->GetId(&device_id));
    hi_assert(device_id);

    auto device_id_ = hi::to_string(device_id);

    CoTaskMemFree(device_id);
    return device_id_;
}

audio_device_win32::audio_device_win32(IMMDevice *device) :
    audio_device(), _previous_state(audio_device_state::uninitialized), _device(device), _audio_client(nullptr)
{
    hi_assert(_device != nullptr);
    _end_point_id = get_device_id(_device);
    _id = std::string{"win32:"} + _end_point_id;
    hi_hresult_check(_device->QueryInterface(&_end_point));
    hi_hresult_check(_device->OpenPropertyStore(STGM_READ, &_property_store));

    EDataFlow data_flow;
    hi_hresult_check(_end_point->GetDataFlow(&data_flow));
    switch (data_flow) {
    case eRender:
        _direction = audio_direction::output;
        break;
    case eCapture:
        _direction = audio_direction::input;
        break;
    case eAll:
        _direction = audio_direction::bidirectional;
        break;
    default:
        hi_no_default();
    }

    update_state();
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

void audio_device_win32::update_state() noexcept
{
    auto new_state = state();

    // Log the correct message.
    if (_previous_state == audio_device_state::uninitialized) {
        hi_log_info(" * Found new audio device '{}' {} ({})", name(), id(), state());

    } else if (_previous_state != new_state) {
        hi_log_info(" * Audio device changed state '{}' {} ({})", name(), id(), state());
    }

    // Start and stop the audio device depending if it was enabled/disabled for some reason.
    if (_previous_state == audio_device_state::active and new_state != audio_device_state::active) {
        _audio_client->Release();
        _audio_client = nullptr;

    } else if (_previous_state != audio_device_state::active and new_state == audio_device_state::active) {
        if (FAILED(_device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, reinterpret_cast<void **>(&_audio_client)))) {
            hi_log_error("Audio device {} does not have IAudioClient interface", name());
            _audio_client = nullptr;
        }

        update_supported_formats();

        // By setting exclusivity to false at the start the audio stream format is initialized properly.
        set_exclusive(false);
    }
    _previous_state = new_state;
}

void audio_device_win32::update_supported_formats() noexcept
{
    // https://stackoverflow.com/questions/50396224/how-to-get-audio-formats-supported-by-physical-device-winapi-windows
    // https://github.com/EddieRingle/portaudio/blob/master/src/os/win/pa_win_wdmks_utils.c
    // https://docs.microsoft.com/en-us/previous-versions/ff561658(v=vs.85)

    try {
        auto wave_device = win32_wave_device::find_matching_end_point(direction(), _end_point_id);
        //auto device_interface = wave_device.open_device_interface();

    } catch (...) {
    }
}

[[nodiscard]] bool audio_device_win32::supports_format(audio_stream_format const& format) const noexcept
{
    auto format_ = audio_stream_format_to_win32(format);

    auto r = _audio_client->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, reinterpret_cast<WAVEFORMATEX *>(&format_), NULL);
    if (r == S_OK) {
        return true;
    } else if (r == S_FALSE or r == AUDCLNT_E_UNSUPPORTED_FORMAT) {
        return false;
    } else {
        hi_log_error("Failed to check format. {}", get_last_error_message());
        return false;
    }
}

[[nodiscard]] bool audio_device_win32::exclusive() const noexcept
{
    return _exclusive;
}

[[nodiscard]] audio_stream_format
audio_device_win32::find_exclusive_stream_format(double sample_rate, hi::speaker_mapping speaker_mapping) noexcept
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

[[nodiscard]] hi::speaker_mapping audio_device_win32::input_speaker_mapping() const noexcept
{
    switch (direction()) {
    case audio_direction::input:
        [[fallthrough]];
    case audio_direction::bidirectional:
        return _speaker_mapping;
    case audio_direction::output:
        return hi::speaker_mapping::none;
    default:
        hi_no_default();
    }
}

void audio_device_win32::set_input_speaker_mapping(hi::speaker_mapping speaker_mapping) noexcept
{
    switch (direction()) {
    case audio_direction::input:
        [[fallthrough]];
    case audio_direction::bidirectional:
        _speaker_mapping = speaker_mapping;
        break;
    case audio_direction::output:
        break;
    default:
        hi_no_default();
    }
}

[[nodiscard]] std::vector<hi::speaker_mapping> audio_device_win32::available_input_speaker_mappings() const noexcept
{
    return {};
}

[[nodiscard]] hi::speaker_mapping audio_device_win32::output_speaker_mapping() const noexcept
{
    switch (direction()) {
    case audio_direction::output:
        [[fallthrough]];
    case audio_direction::bidirectional:
        return _speaker_mapping;
    case audio_direction::input:
        return hi::speaker_mapping::none;
    default:
        hi_no_default();
    }
}

void audio_device_win32::set_output_speaker_mapping(hi::speaker_mapping speaker_mapping) noexcept
{
    switch (direction()) {
    case audio_direction::output:
        [[fallthrough]];
    case audio_direction::bidirectional:
        _speaker_mapping = speaker_mapping;
        break;
    case audio_direction::input:
        break;
    default:
        hi_no_default();
    }
}

[[nodiscard]] std::vector<hi::speaker_mapping> audio_device_win32::available_output_speaker_mappings() const noexcept
{
    return {};
}

/*[[nodiscard]] std::optional<audio_stream_format>
audio_device_win32::best_format(double sample_rate, hi::speaker_mapping speaker_mapping) const noexcept
{
    constexpr auto sample_formats = std::array{
        audio_sample_format::float32(),
        audio_sample_format::fix8_23(),
        audio_sample_format::int24(),
        audio_sample_format::int20(),
        audio_sample_format::int16()};

    for (hilet &sample_format : sample_formats) {
        hilet format = audio_stream_format{sample_format, sample_rate, speaker_mapping};
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

    for (hilet &sample_format : sample_formats) {
        hilet format = audio_stream_format{sample_format, sample_rate, num_channels};
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
    r.reserve(size(speaker_mappings) + (max_num_channels / 2) + 2);

    for (hilet &info: speaker_mappings) {
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
    } catch (io_error const&) {
        return "<unknown name>";
    }
}

hi::label audio_device_win32::label() const noexcept
{
    return {elusive_icon::Speaker, tr("{}", name())};
}

audio_device_state audio_device_win32::state() const noexcept
{
    DWORD state;
    hi_hresult_check(_device->GetState(&state));

    switch (state) {
    case DEVICE_STATE_ACTIVE:
        return audio_device_state::active;
    case DEVICE_STATE_DISABLED:
        return audio_device_state::disabled;
    case DEVICE_STATE_NOTPRESENT:
        return audio_device_state::not_present;
    case DEVICE_STATE_UNPLUGGED:
        return audio_device_state::unplugged;
    default:
        hi_no_default();
    }
}

audio_direction audio_device_win32::direction() const noexcept
{
    return _direction;
}

std::string audio_device_win32::device_name() const noexcept
{
    try {
        return get_property<std::string>(_property_store, PKEY_DeviceInterface_FriendlyName);
    } catch (io_error const&) {
        return "<unknown device name>";
    }
}

std::string audio_device_win32::end_point_name() const noexcept
{
    try {
        return get_property<std::string>(_property_store, PKEY_Device_DeviceDesc);
    } catch (io_error const&) {
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
    hi_hresult_check(_audio_client->GetMixFormat(&ex));
    hi_axiom(ex);
    hilet r = audio_stream_format_from_win32(*ex);
    CoTaskMemFree(ex);
    return r;
}

} // namespace hi::inline v1
