// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_stream_format_win32.hpp"
#include "speaker_mapping_win32.hpp"
#include "../required.hpp"
#include "../exception.hpp"
#include "../cast.hpp"
#include "../check.hpp"
#include <mmeapi.h>
#include <bit>

namespace tt::inline v1 {

[[nodiscard]] WAVEFORMATEXTENSIBLE audio_stream_format_to_win32(audio_stream_format x) noexcept
{
    ttlet sample_rate = narrow_cast<DWORD>(std::round(x.sample_rate));

    bool extended = false;

    // legacy format can only handle mono or stereo.
    extended |= num_channels(x.speaker_mapping) > 2;

    // Legacy format can only handle bits equal to container size.
    extended |= (x.sample_format.num_bytes * 8) != (x.sample_format.num_guard_bits + x.sample_format.num_bits + 1);

    // Legacy format can only handle direct channel map. This allows you to select legacy
    // mono and stereo for old device drivers.
    extended |= x.speaker_mapping != speaker_mapping::direct;

    // Legacy format can only be PCM-8, PCM-16 or PCM-float-32.
    if (x.sample_format.is_float) {
        extended |= x.sample_format.num_bytes == 4;
    } else {
        extended |= x.sample_format.num_bytes <= 2;
    }

    WAVEFORMATEXTENSIBLE r;
    r.Format.wFormatTag = extended ? WAVE_FORMAT_EXTENSIBLE : x.sample_format.is_float ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
    r.Format.nChannels = narrow_cast<WORD>(num_channels(x.speaker_mapping));
    r.Format.nSamplesPerSec = sample_rate;
    r.Format.nAvgBytesPerSec = narrow_cast<DWORD>(sample_rate * num_channels(x.speaker_mapping) * x.sample_format.num_bytes);
    r.Format.nBlockAlign = narrow_cast<WORD>(num_channels(x.speaker_mapping) * x.sample_format.num_bytes);
    r.Format.wBitsPerSample = narrow_cast<WORD>(x.sample_format.num_bytes * 8);
    r.Format.cbSize = extended ? (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)) : 0;
    r.Samples.wValidBitsPerSample = narrow_cast<WORD>(x.sample_format.num_guard_bits + x.sample_format.num_bits + 1);
    r.dwChannelMask = speaker_mapping_to_win32(x.speaker_mapping);
    r.SubFormat = x.sample_format.is_float ? KSDATAFORMAT_SUBTYPE_IEEE_FLOAT : KSDATAFORMAT_SUBTYPE_PCM;
    return r;
}

[[nodiscard]] audio_stream_format audio_stream_format_from_win32(WAVEFORMATEXTENSIBLE const &wave_format)
{
    auto r = audio_stream_format{};

    if (wave_format.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
        r.sample_format.is_float = true;
    } else if (wave_format.SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
        r.sample_format.is_float = false;
    } else {
        throw parse_error("Unknown SubFormat");
    }

    tt_parse_check(wave_format.Format.wBitsPerSample % 8 == 0, "wBitsPerSample is not multiple of 8");
    tt_parse_check(wave_format.Format.wBitsPerSample > 0, "wBitsPerSample is 0");
    tt_parse_check(wave_format.Format.wBitsPerSample <= 32, "wBitsPerSample is more than 32");
    r.sample_format.num_bytes = narrow_cast<uint8_t>(wave_format.Format.wBitsPerSample / 8);

    tt_parse_check(wave_format.Samples.wValidBitsPerSample > 0, "wValidBitsPerSample is 0");
    tt_parse_check(
        wave_format.Samples.wValidBitsPerSample <= wave_format.Format.wBitsPerSample, "wValidBitsPerSample > wBitsPerSample");
    r.sample_format.num_bits = narrow_cast<uint8_t>(wave_format.Samples.wValidBitsPerSample);

    // We do not support win32 fixed-point data format.
    r.sample_format.num_guard_bits = 0;

    // Win32 sample data is always in native endian?
    r.sample_format.endian = std::endian::native;

    r.speaker_mapping = speaker_mapping_from_win32(wave_format.dwChannelMask);
    if (is_direct(r.speaker_mapping)) {
        tt_parse_check(wave_format.Format.nChannels > 0, "nChannels is zero");
        r.speaker_mapping = make_direct_speaker_mapping(wave_format.Format.nChannels);
    } else {
        tt_parse_check(num_channels(r.speaker_mapping) == wave_format.Format.nChannels, "dwChannelMask does not match nChannels");
    }

    tt_parse_check(wave_format.Format.nSamplesPerSec > 0, "nSamplesPerSec is zero");
    r.sample_rate = static_cast<double>(wave_format.Format.nSamplesPerSec);
    return r;
}

[[nodiscard]] audio_stream_format audio_stream_format_from_win32(WAVEFORMATEX const &wave_format)
{
    auto r = audio_stream_format{};

    if (wave_format.wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        if (wave_format.cbSize < (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))) {
            throw parse_error(std::format("WAVEFORMATEXTENSIBLE has incorrect size {}", wave_format.cbSize));
        }
        return audio_stream_format_from_win32(*std::launder(reinterpret_cast<WAVEFORMATEXTENSIBLE const *>(&wave_format)));

    } else if (wave_format.wFormatTag == WAVE_FORMAT_PCM) {
        r.sample_format.is_float = false;

    } else if (wave_format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
        r.sample_format.is_float = true;

    } else {
        throw parse_error(std::format("Unsupported wFormatTag {}", wave_format.wFormatTag));
    }

    tt_parse_check(wave_format.wBitsPerSample > 0, "wBitsPerSample is zero");
    tt_parse_check(wave_format.wBitsPerSample % 8 == 0, "wBitsPerSample is not multiple of 8");
    tt_parse_check(wave_format.wBitsPerSample <= 32, "wBitsPerSample greater than 32");
    r.sample_format.num_bits = narrow_cast<uint8_t>(wave_format.wBitsPerSample);
    r.sample_format.num_guard_bits = 0;
    r.sample_format.num_bytes = narrow_cast<uint8_t>(wave_format.wBitsPerSample / 8);
    r.sample_format.endian = std::endian::native;
    tt_parse_check(wave_format.nSamplesPerSec > 0, "nSamplesPerSec is zero");
    r.sample_rate = static_cast<double>(wave_format.nSamplesPerSec);
    tt_parse_check(wave_format.nChannels > 0, "nChannels is zero");
    r.speaker_mapping = make_direct_speaker_mapping(wave_format.nChannels);
    return r;
}

} // namespace tt::inline v1
