// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/audio_counter_clock.hpp"
#include <Windows.h>
#include <profileapi.h>

namespace TTauri {

using namespace std::chrono_literals;

audio_counter_clock::time_point audio_counter_clock::from_audio_api(uint64_t value) noexcept
{
    // The argument comes from calls such as IAudioCaptureClient::GetBuffer().
    // This value comes from the QueryPerformanceCounter() reference clock after it was
    // adjusted with the QueryPerformanceFrequency() to a number of 100ns intervals.

    // The timepoint is based on a value that is near 1 ns.
    return time_point(value * 100ns);
}

audio_counter_clock::time_point audio_counter_clock::now() noexcept
{
    LARGE_INTEGER counter;

    // Function will always succeed since WindowsXP.
    QueryPerformanceCounter(&counter);

    // XXX Need to use the QueryPerformanceFrequency to get this number near
    // one tick/ns.
    counter.QuadPart *= 100;
    return time_point(duration(counter.QuadPart));
}

}