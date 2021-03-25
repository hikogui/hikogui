

#pragma once

#include "hires_tai_clock.hpp"
#include "time_stamp_count.hpp"

namespace tt {
namespace detail {


std::array<entry_type,64> tsc_to_tai;

}


inline operator hires_tai_clock::timepoint (time_stamp_count tsc) noexcept
{


}


}

class tsc_to_tai {
public:
    constexpr tsc_to_tai() noexcept :
        entries() {}

private:
};


