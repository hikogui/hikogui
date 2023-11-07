// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio_block.hpp"
#include "../macros.hpp"

hi_export_module(hikogui.audio.audio_device_delegate);

hi_export namespace hi { inline namespace v1 {

hi_export class audio_device_delegate {
public:
    audio_device_delegate();
    virtual ~audio_device_delegate() = 0;

    /** Process a block of samples.
     */
    virtual void process_audio() noexcept = 0;
};

}} // namespace hi::inline v1
