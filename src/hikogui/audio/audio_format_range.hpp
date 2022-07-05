// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "pcm_format.hpp"
#include "audio_stream_format.hpp"
#include "surround_mode.hpp"
#include <compare>

namespace hi::inline v1 {

class audio_format_range {
public:
    pcm_format format = {};
    uint16_t num_channels = 0;
    uint32_t min_sample_rate = 0;
    uint32_t max_sample_rate = 0;
    surround_mode surround_mode_mask = surround_mode::none;

    constexpr audio_format_range() noexcept = default;
    constexpr audio_format_range(audio_format_range&&) noexcept = default;
    constexpr audio_format_range(audio_format_range const&) noexcept = default;
    constexpr audio_format_range& operator=(audio_format_range&&) noexcept = default;
    constexpr audio_format_range& operator=(audio_format_range const&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(audio_format_range const&, audio_format_range const&) noexcept = default;

    [[nodiscard]] constexpr friend auto operator<=>(audio_format_range const& lhs, audio_format_range const& rhs) noexcept
    {
        if (auto tmp = lhs.num_channels <=> rhs.num_channels; tmp != std::strong_ordering::equal) {
            return tmp;
        }
        if (auto tmp = lhs.min_sample_rate <=> rhs.min_sample_rate; tmp != std::strong_ordering::equal) {
            return tmp;
        }
        if (auto tmp = lhs.surround_mode_mask <=> rhs.surround_mode_mask; tmp != std::strong_ordering::equal) {
            return tmp;
        }
        return lhs.format <=> rhs.format;
    }

    [[nodiscard]] constexpr friend bool
    equal_except_bit_depth(audio_format_range const& lhs, audio_format_range const& rhs) noexcept
    {
        return std::tie(lhs.num_channels, lhs.min_sample_rate, lhs.max_sample_rate, lhs.surround_mode_mask) ==
            std::tie(rhs.num_channels, rhs.min_sample_rate, rhs.max_sample_rate, rhs.surround_mode_mask) and
            equal_except_bit_depth(lhs.format, rhs.format);
    }

    constexpr audio_format_range(
        pcm_format format,
        uint16_t num_channels,
        uint32_t min_sample_rate,
        uint32_t max_sample_rate,
        surround_mode surround_mode_mask) noexcept :
        format(format),
        num_channels(num_channels),
        min_sample_rate(min_sample_rate),
        max_sample_rate(max_sample_rate),
        surround_mode_mask(surround_mode_mask)
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
            "format={}, ch={}, rate={}:{}, surround={}",
            rhs.format,
            rhs.num_channels,
            rhs.min_sample_rate,
            rhs.max_sample_rate,
            rhs.surround_mode_mask);
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
