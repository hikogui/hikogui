// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "speaker_mapping.hpp"

namespace hi::inline v1 {

[[nodiscard]] speaker_mapping speaker_mapping_from_win32(DWORD from);

[[nodiscard]] DWORD speaker_mapping_to_win32(speaker_mapping from) noexcept;

} // namespace hi::inline v1
