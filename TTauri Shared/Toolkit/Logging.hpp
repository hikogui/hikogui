//
//  Logging.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-07.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshorten-64-to-32"

#include <boost/log/trivial.hpp>
#include <boost/format.hpp>
#pragma clang diagnostic pop

namespace TTauri {
namespace Toolkit {


}}

#define LOG_TRACE(fmt) BOOST_LOG_TRIVIAL(trace) << boost::format(fmt)
#define LOG_DEBUG(fmt) BOOST_LOG_TRIVIAL(debug) << boost::format(fmt)
#define LOG_INFO(fmt) BOOST_LOG_TRIVIAL(info) << boost::format(fmt)
#define LOG_WARNING(fmt) BOOST_LOG_TRIVIAL(warning) << boost::format(fmt)
#define LOG_ERROR(fmt) BOOST_LOG_TRIVIAL(error) << boost::format(fmt)
#define LOG_FATAL(fmt) BOOST_LOG_TRIVIAL(fatal) << boost::format(fmt)
