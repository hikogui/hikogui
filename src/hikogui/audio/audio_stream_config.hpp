// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../macros.hpp"

hi_export_module(hikogui.audio.audio_stream_config);

hi_export namespace hi { inline namespace v1 {

hi_export struct audio_stream_config {
    double sample_rate;
};

}} // namespace hi::inline v1
