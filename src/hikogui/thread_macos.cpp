// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "thread.hpp"
#include "strings.hpp"
#include "application.hpp"

#include <pthread.h>

namespace hi::inline v1 {

static std::atomic<uint32_t> thread_id_count = 0;

void set_thread_name(std::string_view name)
{
    pthread_setname_np(name.data());
}

} // namespace hi::inline v1
