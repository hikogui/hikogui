// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_stream_format.hpp"

namespace hi::inline v1 {

[[nodiscard]] bool win32_use_extensible(audio_stream_format x) noexcept;
[[nodiscard]] WAVEFORMATEXTENSIBLE audio_stream_format_to_win32(audio_stream_format stream_format, bool extensible) noexcept;
[[nodiscard]] audio_stream_format audio_stream_format_from_win32(WAVEFORMATEXTENSIBLE const& wave_format);
[[nodiscard]] audio_stream_format audio_stream_format_from_win32(WAVEFORMATEX const& wave_format);

} // namespace hi::inline v1
