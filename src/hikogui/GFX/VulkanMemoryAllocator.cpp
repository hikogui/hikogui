// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../architecture.hpp"
#include "../required.hpp"

hi_warning_push();
hi_clang_suppress("-Wunused-variable");
hi_clang_suppress("-Wreorder");
hi_clang_suppress("-Wunused-private-field");

hi_msvc_suppress(4127);
hi_msvc_suppress(6011);
hi_msvc_suppress(6386);
hi_msvc_suppress(6387);
hi_msvc_suppress(4701);
hi_msvc_suppress(4703);
hi_msvc_suppress(4189);

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

hi_warning_pop();
