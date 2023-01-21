// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../tagged_id.hpp"
#include <cstdint>

namespace hi {
inline namespace v1 {

using widget_id = tagged_id<uint32_t, "widget">;

}}
