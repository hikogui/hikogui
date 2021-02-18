// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../os_detect.hpp"

clang_suppress("-Wunused-variable")
clang_suppress("-Wreorder")
clang_suppress("-Wunused-private-field")

msvc_suppress(4127)
msvc_suppress(6011)
msvc_suppress(6386)
msvc_suppress(6387)
msvc_suppress(4701)
msvc_suppress(4703)
msvc_suppress(4189)

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
