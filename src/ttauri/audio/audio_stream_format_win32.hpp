// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_stream_format.hpp"

namespace tt {

[[nodiscard]] WAVEFORMATEXTENSIBLE audio_stream_format_to_win32(audio_stream_format stream_format) noexcept;

}
