// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"
#include "../win32_headers.hpp"


#include <format>

export module hikogui_random_seed : impl;
import hikogui_utility;
import : intf;

namespace hi::inline v1 {

inline void generate_seed(void *ptr, size_t size)
{
    auto status = BCryptGenRandom(NULL, static_cast<PUCHAR>(ptr), narrow_cast<ULONG>(size), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (not SUCCEEDED(status)) {
        throw os_error(std::format("BCryptGenRandom(): {}", get_last_error_message(status)));
    }
}

}
