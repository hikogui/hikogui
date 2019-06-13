// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <boost/format.hpp>
#include <boost/log/trivial.hpp>
#include <string>

namespace TTauri {

#ifdef _WIN32
std::string getLastErrorMessage();
#endif

void initializeLogging();

template<typename F>
inline std::string sformat(F format)
{
    return format;
}

template<typename F, typename... Targs>
inline std::string sformat(F format, Targs... Fargs)
{
    return (boost::format(format) % ... % Fargs).str();
}

}

#define LOG_TRACE(...) BOOST_LOG_TRIVIAL(trace) << ::TTauri::sformat(__VA_ARGS__)
#define LOG_DEBUG(...) BOOST_LOG_TRIVIAL(debug) << ::TTauri::sformat(__VA_ARGS__)
#define LOG_INFO(...) BOOST_LOG_TRIVIAL(info) << ::TTauri::sformat(__VA_ARGS__)
#define LOG_WARNING(...) BOOST_LOG_TRIVIAL(warning) << ::TTauri::sformat(__VA_ARGS__)
#define LOG_ERROR(...) BOOST_LOG_TRIVIAL(error) << ::TTauri::sformat(__VA_ARGS__)
#define LOG_FATAL(...) BOOST_LOG_TRIVIAL(fatal) << ::TTauri::sformat(__VA_ARGS__); std::abort()

