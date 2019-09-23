// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Diagnostic/logger.hpp"
#include "TTauri/Diagnostic/trace.hpp"
#include "TTauri/Time/cpu_utc_clock.hpp"
#include "TTauri/Time/globals.hpp"
#include "TTauri/Required/required.hpp"
#include "TTauri/Required/URL.hpp"
#include "TTauri/Required/strings.hpp"
#include "TTauri/Required/thread.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <exception>
#include <memory>
#include <iostream>
#include <ostream>
#include <chrono>

#import <Foundation/Foundation.h>

namespace TTauri {

using namespace std::literals::chrono_literals;

void logger_type::writeToConsole(std::string str) noexcept {
    @autoreleasepool {
        NSString *tmp = [NSString stringWithCString:str.data() encoding:NSUTF8StringEncoding];
        NSLog(@"%@", tmp);
    }
}

}
