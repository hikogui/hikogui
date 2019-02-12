//
//  Time.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <cstdint>

namespace TTauri {
namespace Toolkit {

/*! Timestamp
 * The timestamp is defined as the number of nanoseconds since 1970-01-01 00:00:00.000000000 on
 * the TAI time standard. This is the same format for PTP timestamps which is also the recommendation
 * for processing time for audio/video by SMPTE.
 */
struct Timestamp {
    int64_t intrinsic;
};


}}
