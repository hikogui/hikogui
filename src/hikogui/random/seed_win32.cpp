// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../win32_headers.hpp"

#include "seed.hpp"
#include "../exception.hpp"
#include "../log.hpp"
#include "../cast.hpp"

namespace hi::inline v1 {

void generate_seed(void *ptr, size_t size)
{
    auto status = BCryptGenRandom(NULL, reinterpret_cast<PUCHAR>(ptr), narrow_cast<ULONG>(size), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (not SUCCEEDED(status)) {
        hi_log_error("BCryptGenRandom(): {}", get_last_error_message());
        throw os_error("BCryptGenRandom()");
    }
}

}
