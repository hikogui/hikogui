// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "audio_device_id.hpp"
#include "../unicode/UTF.hpp"
#include "../log.hpp"

namespace hi::inline v1 {

audio_device_id::audio_device_id(char type, wchar_t const *id) noexcept : _v{}
{
    hi_axiom(id);
    hi_axiom(type == audio_device_id::win32);

    auto id_ = hi::to_string(id);
    _v[0] = type;

    auto id_size_with_nul = std::min(size(id_) + 1, size(_v) - 1);
    std::memcpy(&_v[1], id_.c_str(), id_size_with_nul);

    if (size(id_) > size(_v) - 1) {
        hi_log_error("Audio device id is too large '{}'.", id_);
    }
}

} // namespace hi::inline v1
