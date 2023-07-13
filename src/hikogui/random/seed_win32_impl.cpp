// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../utility/win32_headers.hpp"

#include "seed.hpp"
#include "../utility/module.hpp"
#include <format>

namespace hi::inline v1 {

void generate_seed(void *ptr, size_t size)
{
    auto status = BCryptGenRandom(NULL, static_cast<PUCHAR>(ptr), narrow_cast<ULONG>(size), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (not SUCCEEDED(status)) {
        throw os_error(std::format("BCryptGenRandom(): {}", get_last_error_message(status)));
    }
}

}
