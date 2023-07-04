// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


#include "security.hpp"
#include <windows.h>

namespace tt::inline v1 {


void secure_clear(void *ptr, size_t size) noexcept
{
    SecureZeroMemory(ptr, size);
}

}

