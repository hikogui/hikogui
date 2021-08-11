// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_sample_format.hpp"
#include "speaker_mapping.hpp"
#include <bit>

namespace tt {

/** The format of a stream of audio.
 */
struct audio_stream_format {
    audio_sample_format sample_format;
    double sample_rate;
    tt::speaker_mapping speaker_mapping;

    constexpr audio_stream_format() noexcept :
        sample_format(), sample_rate(0.0f), speaker_mapping(tt::speaker_mapping::direct) {}

    constexpr audio_stream_format(audio_stream_format const &) noexcept = default;
    constexpr audio_stream_format(audio_stream_format &&) noexcept = default;
    constexpr audio_stream_format &operator=(audio_stream_format const &) noexcept = default;
    constexpr audio_stream_format &operator=(audio_stream_format &&) noexcept = default;

    [[nodiscard]] constexpr audio_stream_format(
        audio_sample_format sample_format,
        double sample_rate,
        tt::speaker_mapping speaker_mapping) noexcept :
        sample_format(sample_format),
        sample_rate(sample_rate),
        speaker_mapping(speaker_mapping)
    {
    }

    constexpr explicit operator bool () const noexcept
    {
        return static_cast<bool>(sample_format);
    }
};

} // namespace tt
