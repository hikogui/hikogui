// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "security_intf.hpp"
#include "../macros.hpp"
#include <windows.h>

hi_export_module(hikogui.security : impl);

namespace tt::inline v1 {


hi_inline void secure_clear(void *ptr, size_t size) noexcept
{
    SecureZeroMemory(ptr, size);
}

}

