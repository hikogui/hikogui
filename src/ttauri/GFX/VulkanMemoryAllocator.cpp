// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../architecture.hpp"

clang_suppress("-Wunused-variable") clang_suppress("-Wreorder") clang_suppress("-Wunused-private-field")

    tt_msvc_pragma("warning(disable:4127)") tt_msvc_pragma("warning(disable:6011)") tt_msvc_pragma("warning(disable:6386)")
        tt_msvc_pragma("warning(disable:6387)") tt_msvc_pragma("warning(disable:4701)") tt_msvc_pragma("warning(disable:4703)")
            tt_msvc_pragma("warning(disable:4189)")

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
