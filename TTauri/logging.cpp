// Copyright 2019 Pokitec
// All rights reserved.

#include "logging.hpp"
#include "strings.hpp"

#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/sinks/debug_output_backend.hpp>
#include <boost/exception/all.hpp>
#include <exception>
#include <memory>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace TTauri {

#ifdef _WIN32
typedef boost::log::sinks::synchronous_sink< boost::log::sinks::debug_output_backend > sink_t;

void initializeLogging() noexcept
{
    boost::shared_ptr< boost::log::core > core = boost::log::core::get();

    // Create the sink. The backend requires synchronization in the frontend.
    boost::shared_ptr< sink_t > sink = boost::make_shared<sink_t>();

    // Set the special filter to the frontend
    // in order to skip the sink when no debugger is available
    sink->set_filter(boost::log::expressions::is_debugger_present());
    sink->set_formatter
    (
        boost::log::expressions::format("%1%: [%2%] - %3%\r\n")
        % boost::log::expressions::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d %H:%M:%S")
        % boost::log::trivial::severity
        % boost::log::expressions::smessage
    );

    core->add_sink(sink);
}

gsl_suppress(i.11)
std::string getLastErrorMessage()
{
    DWORD const errorCode = GetLastError();
    size_t const messageSize = 32768;
    wchar_t* const c16_message = new wchar_t[messageSize];;

    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, // source
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        c16_message,
        messageSize,
        NULL
    );

    let message = translateString<std::string>(std::wstring(c16_message));
    delete [] c16_message;

    return message;
}

#else
void initializeLogging()
{
}
#endif

}
 
