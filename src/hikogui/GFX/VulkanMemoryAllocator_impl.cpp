// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../utility/module.hpp"

hi_warning_push();
hi_warning_ignore_clang("-Wunused-variable");
hi_warning_ignore_clang("-Wreorder");
hi_warning_ignore_clang("-Wunused-private-field");

hi_warning_ignore_msvc(4127);
hi_warning_ignore_msvc(6011);
hi_warning_ignore_msvc(6386);
hi_warning_ignore_msvc(6387);
hi_warning_ignore_msvc(4701);
hi_warning_ignore_msvc(4703);
hi_warning_ignore_msvc(4189);

#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

hi_warning_pop();
