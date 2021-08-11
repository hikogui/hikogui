// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_device_id.hpp"
#include "../codec/UTF.hpp"
#include "../logger.hpp"

namespace tt {

audio_device_id::audio_device_id(char type, wchar_t const *id) noexcept
{
    tt_axiom(id);
    tt_axiom(type == audio_device_id::win32);

    auto out_it = std::begin(_v);
    ttlet out_end = std::cend(_v);

    while (out_it != out_end) {
        auto c32 = utf16_to_utf32(id, nullptr);
        utf32_to_utf8(c32, out_it, out_end);

        if (c32 == 0) {
            return;
        }
    }
    tt_log_error("Audio device id is too large '{}'.", to_string(std::wstring(id)));
}


}