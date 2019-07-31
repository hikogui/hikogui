// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "hires_utc_clock.hpp"
#include "hires_tai_clock.hpp"

namespace TTauri::Time {

/*! Create a tai clock from a utc-clock.
 */
class LeapSecondDB {
    LeapSecondDB() {
        
    }

};

LeapSecondDB parseLeapSecondDB(gsl::span<std::byte const> bytes);

}

namespace TTauri {

template<>
inline Time::LeapSecondDB parseResource(URL const &location)
{
    let view = ResourceView(location);

    if (location.extension() == "list") {
        try {
            return Time::parseLeapSecondDB(view.bytes());
        } catch (boost::exception &e) {
            e << errinfo_url(location);
            throw;
        }

    } else {
        BOOST_THROW_EXCEPTION(FileError("Unknown extension")
            << errinfo_url(location)
        );
    }
}

}
