// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <fmt/format.h>
#include <boost/log/trivial.hpp>
#include <string>

namespace TTauri {

#ifdef _WIN32
std::string getLastErrorMessage();
#endif

void initializeLogging() noexcept;

}

#define LOG_TRACE(...) BOOST_LOG_TRIVIAL(trace) << fmt::format(__VA_ARGS__)
#define LOG_DEBUG(...) BOOST_LOG_TRIVIAL(debug) << fmt::format(__VA_ARGS__)
#define LOG_INFO(...) BOOST_LOG_TRIVIAL(info) << fmt::format(__VA_ARGS__)
#define LOG_WARNING(...) BOOST_LOG_TRIVIAL(warning) << fmt::format(__VA_ARGS__)
#define LOG_ERROR(...) BOOST_LOG_TRIVIAL(error) << fmt::format(__VA_ARGS__)
#define LOG_FATAL(...) BOOST_LOG_TRIVIAL(fatal) << fmt::format(__VA_ARGS__); std::abort()

