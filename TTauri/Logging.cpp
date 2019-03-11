//
//  Logging.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-07.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "Logging.hpp"
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

namespace TTauri {

#ifdef _WIN32
typedef boost::log::sinks::synchronous_sink< boost::log::sinks::debug_output_backend > sink_t;

void initializeLogging()
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
#else
void initializeLogging()
{
}
#endif

}
 
