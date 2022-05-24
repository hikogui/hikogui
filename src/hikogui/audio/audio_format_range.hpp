// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pcm_format.hpp"
#include "audio_stream_format.hpp"

namespace hi::inline v1 {

class audio_format_range {
public:
    pcm_format format = {};
    uint32_t min_sample_rate = 0;
    uint32_t max_sample_rate = 0;
    uint16_t min_channels = 0;
    uint16_t max_channels = 0;

    constexpr audio_format_range() noexcept = default;
    constexpr audio_format_range(audio_format_range&&) noexcept = default;
    constexpr audio_format_range(audio_format_range const&) noexcept = default;
    constexpr audio_format_range& operator=(audio_format_range&&) noexcept = default;
    constexpr audio_format_range& operator=(audio_format_range const&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(audio_format_range const&, audio_format_range const&) noexcept = default;

    constexpr audio_format_range(
        pcm_format format,
        uint32_t min_sample_rate,
        uint32_t max_sample_rate,
        uint16_t min_channels,
        uint16_t max_channels) noexcept :
        format(format),
        min_sample_rate(min_sample_rate),
        max_sample_rate(max_sample_rate),
        min_channels(min_channels),
        max_channels(max_channels)
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

    [[nodiscard]] friend std::string to_string(audio_format_range const& rhs) noexcept
    {
        return std::format(
            "format={}, ch={}:{}, rate={}:{}",
            rhs.format,
            rhs.min_channels,
            rhs.max_channels,
            rhs.min_sample_rate,
            rhs.max_sample_rate);
    }

    /** Get a format matching this range that is likely working.
     *
     * The audio device driver was probably lying about its capabilities. This
     * function returns a format that most likely will actually work.
     *
     * @return A stream-format with the minimum sample rate and maximum channels.
     */
    [[nodiscard]] audio_stream_format simple_format() const noexcept
    {
        return audio_stream_format(format, min_sample_rate, make_direct_speaker_mapping(max_channels));
    }
};

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::audio_format_range, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::audio_format_range const& t, auto& fc)
    {
        return std::formatter<std::string_view, CharT>::format(to_string(t), fc);
    }
};
