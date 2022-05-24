// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../win32_headers.hpp"

#include "audio_stream_format_win32.hpp"
#include "speaker_mapping_win32.hpp"
#include "../required.hpp"
#include "../exception.hpp"
#include "../cast.hpp"
#include "../check.hpp"
#include <bit>

namespace hi::inline v1 {

[[nodiscard]] static bool win32_use_extensible(audio_stream_format x) noexcept
{
    if (num_channels(x.speaker_mapping) > 2) {
        // According to the specification we SHOULD use extensible with more than two channels.
        // However technically there is no reason to do so.
        // And not all audio device drivers can handle extensible.
        return true;
    }

    if (x.format.num_bytes() * 8 != x.format.num_bits()) {
        // According to the specification we SHOULD use extensible when less bits are used.
        // However technically there is no reason to do so.
        // And not all audio device drivers can handle extensible.
        return true;
    }

    if (not is_direct(x.speaker_mapping)) {
        // If we have an non-direct speaker mapping we MUST use extensible as it requires the
        // extra dwChannelMask field.
        return true;
    }

    return false;
}

[[nodiscard]] WAVEFORMATEXTENSIBLE audio_stream_format_to_win32(audio_stream_format x) noexcept
{
    WAVEFORMATEXTENSIBLE r;

    if (win32_use_extensible(x)) {
        r.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
        // difference between sizeof(WAVEFORMATEXT) and sizeof(WAVEFORMATEXTENSIBLE).
        // But API documentation says it must be "22".
        r.Format.cbSize = 22;
    } else {
        r.Format.wFormatTag = x.format.floating_point() ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
        r.Format.cbSize = 0;
    }

    // These are the fields of WAVEFORMATEXT.
    r.Format.nChannels = narrow_cast<WORD>(num_channels(x.speaker_mapping));
    r.Format.nSamplesPerSec = narrow_cast<DWORD>(std::round(x.sample_rate));
    r.Format.nAvgBytesPerSec = narrow_cast<DWORD>(x.sample_rate * num_channels(x.speaker_mapping) * x.format.num_bytes());
    r.Format.nBlockAlign = narrow_cast<WORD>(num_channels(x.speaker_mapping) * x.format.num_bytes());
    r.Format.wBitsPerSample = narrow_cast<WORD>(x.format.num_bytes() * 8);

    // These are the fields of WAVEFORMATEXTENSIBLE which are ignored for WAVEFORMATEXT.
    r.Samples.wValidBitsPerSample = narrow_cast<WORD>(x.format.num_bits());
    r.dwChannelMask = speaker_mapping_to_win32(x.speaker_mapping);
    r.SubFormat = x.format.floating_point() ? KSDATAFORMAT_SUBTYPE_IEEE_FLOAT : KSDATAFORMAT_SUBTYPE_PCM;
    return r;
}

[[nodiscard]] audio_stream_format audio_stream_format_from_win32(WAVEFORMATEXTENSIBLE const& wave_format)
{
    auto r = audio_stream_format{};

    hi_parse_check(wave_format.Format.wBitsPerSample % 8 == 0, "wBitsPerSample is not multiple of 8");
    hi_parse_check(wave_format.Format.wBitsPerSample > 0, "wBitsPerSample is 0");
    hi_parse_check(wave_format.Format.wBitsPerSample <= 32, "wBitsPerSample is more than 32");
    hi_parse_check(wave_format.Samples.wValidBitsPerSample > 0, "wValidBitsPerSample is 0");
    hi_parse_check(
        wave_format.Samples.wValidBitsPerSample <= wave_format.Format.wBitsPerSample, "wValidBitsPerSample > wBitsPerSample");

    if (wave_format.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) {
        hi_parse_check(wave_format.Format.wBitsPerSample == 32, "wBitsPerSample is not 32");
        r.format = pcm_format{true, std::endian::native, true, 4, 8, 23};

    } else if (wave_format.SubFormat == KSDATAFORMAT_SUBTYPE_PCM) {
        hilet num_bytes = narrow<uint8_t>(wave_format.Format.wBitsPerSample / 8);
        hilet num_minor_bits = narrow<uint8_t>(wave_format.Samples.wValidBitsPerSample - 1);
        r.format = pcm_format{false, std::endian::native, true, num_bytes, 0, num_minor_bits};
    } else {
        throw parse_error("Unknown SubFormat");
    }

    r.speaker_mapping = speaker_mapping_from_win32(wave_format.dwChannelMask);
    if (is_direct(r.speaker_mapping)) {
        hi_parse_check(wave_format.Format.nChannels > 0, "nChannels is zero");
        r.speaker_mapping = make_direct_speaker_mapping(wave_format.Format.nChannels);
    } else {
        hi_parse_check(num_channels(r.speaker_mapping) == wave_format.Format.nChannels, "dwChannelMask does not match nChannels");
    }

    hi_parse_check(wave_format.Format.nSamplesPerSec > 0, "nSamplesPerSec is zero");
    r.sample_rate = narrow<uint32_t>(wave_format.Format.nSamplesPerSec);
    return r;
}

[[nodiscard]] audio_stream_format audio_stream_format_from_win32(WAVEFORMATEX const& wave_format)
{
    auto r = audio_stream_format{};
    hi_parse_check(wave_format.wBitsPerSample > 0, "wBitsPerSample is zero");
    hi_parse_check(wave_format.wBitsPerSample % 8 == 0, "wBitsPerSample is not multiple of 8");
    hi_parse_check(wave_format.wBitsPerSample <= 32, "wBitsPerSample greater than 32");
    hi_parse_check(wave_format.nSamplesPerSec > 0, "nSamplesPerSec is zero");
    hi_parse_check(wave_format.nChannels > 0, "nChannels is zero");

    if (wave_format.wFormatTag == WAVE_FORMAT_EXTENSIBLE) {
        if (wave_format.cbSize < (sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX))) {
            throw parse_error(std::format("WAVEFORMATEXTENSIBLE has incorrect size {}", wave_format.cbSize));
        }
        return audio_stream_format_from_win32(*std::launder(reinterpret_cast<WAVEFORMATEXTENSIBLE const *>(&wave_format)));

    } else if (wave_format.wFormatTag == WAVE_FORMAT_PCM) {
        hi_parse_check(wave_format.wBitsPerSample == 32, "wBitsPerSample is not 32");
        r.format = pcm_format{true, std::endian::native, true, 4, 8, 23};

    } else if (wave_format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT) {
        hilet num_bytes = narrow<uint8_t>(wave_format.wBitsPerSample / 8);
        hilet num_minor_bits = narrow<uint8_t>(wave_format.wBitsPerSample);
        r.format = pcm_format{false, std::endian::native, true, num_bytes, 0, num_minor_bits};

    } else {
        throw parse_error(std::format("Unsupported wFormatTag {}", wave_format.wFormatTag));
    }

    r.sample_rate = narrow<uint32_t>(wave_format.nSamplesPerSec);
    r.speaker_mapping = make_direct_speaker_mapping(wave_format.nChannels);
    return r;
}

} // namespace hi::inline v1
