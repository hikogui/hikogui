// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_device.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.audio.audio_device_asio);

hi_export namespace hi { inline namespace v1 {

/** A class representing an audio device on the system.
 */
hi_export class audio_device_asio : public audio_device {
};

}} // namespace hi::inline v1
