// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <cstdint>
#include <string>
#include <thread>
#include <functional>

namespace TTauri {

struct RequiredGlobals;
inline RequiredGlobals *Required_globals = nullptr;

struct RequiredGlobals {
public:
    std::thread::id main_thread_id;
    std::string applicationName;
    std::function<void(std::function<void()>)> main_thread_runner;
    std::function<void(char const *, int, char const *)> assert_logger;

    RequiredGlobals(std::thread::id main_thread_id, std::string applicationName);
    ~RequiredGlobals();
    RequiredGlobals(RequiredGlobals const &) = delete;
    RequiredGlobals &operator=(RequiredGlobals const &) = delete;
    RequiredGlobals(RequiredGlobals &&) = delete;
    RequiredGlobals &operator=(RequiredGlobals &&) = delete;
};

}