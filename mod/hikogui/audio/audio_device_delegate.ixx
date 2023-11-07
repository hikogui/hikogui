// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"


export module hikogui_audio_audio_device_delegate;
import hikogui_audio_audio_block;

export namespace hi { inline namespace v1 {

export class audio_device_delegate {
public:
    audio_device_delegate();
    virtual ~audio_device_delegate() = 0;

    /** Process a block of samples.
     */
    virtual void process_audio() noexcept = 0;
};

}} // namespace hi::inline v1
