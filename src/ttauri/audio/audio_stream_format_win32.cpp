// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_stream_format_win32.hpp"
#include "../required.hpp"
#include "../exception.hpp"
#include <mmeapi.h>
#include <bit>

namespace tt {

[[nodiscard]] WAVEFORMATEXTENSIBLE audio_stream_format_to_win32(audio_stream_format stream_format x) noexcept
{
    tt_axiom(std::popcount<x.speaker_mapping> <= x.num_channels);

    ttlet sample_rate = narrow_cast<DWORD>(std::round(sample_rate));

    bool extended = false;

    // legacy format can only handle mono or stereo.
    extended |= x.num_channels > 2;

    // Legacy format can only handle bits equal to container size.
    extended |= (x.sample_format.num_bytes * 8) != (x.sample_format.num_guard_bits + x.sample_format.num_bits + 1);

    // Legacy format can only handle direct channel map. This allows you to select legacy
    // mono and stereo for old device drivers.
    extended |= x.speaker_mapping != speaker_mapping::direct;

    // Legacy format can only be PCM-8, PCM-16 or PCM-float-32.
    if (x.sameple_format.is_float) {
        extended |= x.sameple_format.num_bytes == 4;
    } else {
        extended |= x.sameple_format.num_bytes <= 2;
    }

    WAVEFORMATEXTENSIBLE r;
    r.Format.wFormatTag = extended ? WAVE_FORMAT_EXTENSIBLE : x.sample_format.is_float ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
    r.Format.nChannels = x.num_channels;
    r.Format.nSamplesPerSec = sample_rate;
    r.Format.nAvgBytesPerSec = narrow_cast<DWORD>(sample_rate * x.num_channels * x.sample_format.num_bytes);
    r.Format.nBlockAlign = narrow_cast<WORD>(x.num_channels * x.sample_format.num_bytes);
    r.Format.wBitsPerSample = narrow_cast<WORD>(x.sample_format.num_bytes * 8);
    r.Format.cbSize = extended ? (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)) : 0;
    r.Samples.wValidBitsPerSample = narrow_cast<WORD>(x.sample_format.num_guard_bits + x.sample_format.num_bits + 1);
    r.dwChannelMask = speaker_mapping_to_win32(x.speaker_format);
    r.SubFormat = x.sample_format.is_foat ? KSDATAFORMAT_SUBTYPE_IEEE_FLOAT : KSDATAFORMAT_SUBTYPE_PCM;
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
        throw parse_error("Unknown SubFormat {}", wave_format.SubFormat);
    }

    r.sample_format.num_guard_bits = 0;
    if (wave_format.wValidBitsPerSample > 0 and wave_format.wValidBitsPerSample <= wave_format.Format.wValidBitsPerSample) {
        throw parse_error("Invalid valid-bits per sample {}", wave_format.wave_format.wValidBitsPerSample);
    }

    r.sample_format.num_bits = wave_format.Format.wValidBitsPerSample;
    if (wave_format.wBitsPerSample % 8 != 0) {
        throw parse_error("Invalid bits per sample {}", wave_format.wBitsPerSample);
    }

    r.sample_format.num_bytes = wave_format.Format.wBitsPerSample / 8;
    r.sample_format.endian = std::endian::native;

    r.speaker_mapping = speaker_mapping_from_win32(wave_format.dwChannelMask);
    if (r.speaker_mapping == speaker_mapping::direct) {
        r.speaker_mapping = make_direct_speaker_mapping(wave_format.Format.nChannels);

    } else if (num_channels(r.speaker_mapping) != wave_format.Format.nChannels) {
        throw parse_error(
            "Speaker mapping {} does not match number of channels {}", r.speaker_mapping, wave_format.Format.nChannels);
    }

    r.sample_rate = static_cast<double>(wave_format.Format.nSamplesPerSec);
    return r;
}

[[nodiscard]] audio_stream_format audio_stream_format_from_win32(WAVEFORMATEX const &wave_format)
{
    auto r = audio_stream_format{};

    if (wave_format.wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        if (wave_format.cbSize < (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))) {
            throw parse_error("WAVEFORMATEXTENSIBLE has incorrect size {}", wave_format.cbSize);
        }
        return audio_stream_format_from_win32(*std::launder(reinterpret_cast<WAVEFORMATEXTENSIBLE *>(&wave_format)));

    } else if (wave_format.wFormatTag == WAVE_FORMAT_PCM) {
        r.sample_format.is_float = false;

    } else if (wave_format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
        r.sample_format.is_float = true;

    } else {
        throw parse_error("Unknown wFormatTag {}", wave_format.wFormatTag);
    }

    r.sample_format.num_guard_bits = 0;
    r.sample_format.num_bits = wave_format.wBitsPerSample;
    if (wave_format.wBitsPerSample % 8 != 0) {
        throw parse_error("Invalid bits per sample {}", wave_format.wBitsPerSample);
    }
    r.sample_format.num_bytes = wave_format.wBitsPerSample / 8;
    r.sample_format.endian = std::endian::native;
    r.sample_rate = static_cast<double>(wave_format.nSamplesPerSec);
    r.speaker_mapping = make_direct_speaker_mapping(wave_format.nChannels);
    return r;
}
