// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../architecture.hpp"

tt_warning_push();
tt_clang_suppress("-Wunused-variable");
tt_clang_suppress("-Wreorder");
tt_clang_suppress("-Wunused-private-field");

tt_msvc_suppress(4127);
tt_msvc_suppress(6011);
tt_msvc_suppress(6386);
tt_msvc_suppress(6387);
tt_msvc_suppress(4701);
tt_msvc_suppress(4703);
tt_msvc_suppress(4189);

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

tt_warning_pop();
