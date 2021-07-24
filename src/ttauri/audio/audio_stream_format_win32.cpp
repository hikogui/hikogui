// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_stream_format_win32.hpp"
#include "../required.hpp"
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
    if (x.sameple_format.is_float)
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
    r.SubFormat = x.sample_format.is_foat ? KSDATAFORMAT_SUBTYPE_IEEE_FLOAT :  KSDATAFORMAT_SUBTYPE_PCM;
    return r;

}

}

