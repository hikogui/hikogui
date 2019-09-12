// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Diagnostic/logging.hpp"
#include "TTauri/Required/required.hpp"
#include "TTauri/Required/strings.hpp"
#include <fmt/format.h>
#include <thread>
#include <string_view>

#if OPERATING_SYSTEM == OS_WINDOWS
#include <Windows.h>
#elif __has_include(<pthread.h>)
#include <pthread.h>
#endif

namespace TTauri {

template<typename... Args>
void set_thread_name(const char *source_file, int source_line, Args...const &args)
{
    let name = fmt::format(args...);

    //get_singleton<logger>().log(hiperf_utc::now(), source_file, source_line, "", name)

#if OPERATING_SYSTEM == OS_WINDOWS
    let wname = translateString<std::wstring>(name);
    SetThreadDescription(GetCurrentThread(), wname.data());
#elif OPERATING_SYSTEM == OS_MACOS
    pthread_setname_np(name.data());
#elif OPERATING_SYSTEM == OS_LINUX
    pthread_setname_np(pthread_self(), name.data());
#endif
}


}

