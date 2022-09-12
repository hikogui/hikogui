// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pcm_format.hpp"
#include "speaker_mapping.hpp"
#include <bit>

namespace hi::inline v1 {

/** The format of a stream of audio.
 */
struct audio_stream_format {
    pcm_format format = {};
    uint32_t sample_rate = 0;
    uint16_t num_channels = 0;
    hi::speaker_mapping speaker_mapping = hi::speaker_mapping::none;

    constexpr audio_stream_format() noexcept = default;
    constexpr audio_stream_format(audio_stream_format const &) noexcept = default;
    constexpr audio_stream_format(audio_stream_format &&) noexcept = default;
    constexpr audio_stream_format &operator=(audio_stream_format const &) noexcept = default;
    constexpr audio_stream_format &operator=(audio_stream_format &&) noexcept = default;

    [[nodiscard]] constexpr audio_stream_format(
        pcm_format format,
        uint32_t sample_rate,
        uint16_t num_channels,
        hi::speaker_mapping speaker_mapping = hi::speaker_mapping::none) noexcept :
        format(format), sample_rate(sample_rate), num_channels(num_channels), speaker_mapping(speaker_mapping)
    {
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return format.empty();
    }

    constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        if (popcount(speaker_mapping) != 0 and num_channels != popcount(speaker_mapping)) {
            return false;
        }
        return true;
    }
};

} // namespace hi::inline v1
