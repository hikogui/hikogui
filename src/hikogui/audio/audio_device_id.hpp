// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../pickle.hpp"
#include "../exception.hpp"
#include "../check.hpp"
#include "../long_tagged_id.hpp"
#include <array>

namespace hi::inline v1 {

using audio_device_id = long_tagged_id<"audio_device_id">;

} // namespace hi::inline v1
