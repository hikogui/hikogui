// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "clock_counter.hpp"
#include "os_detect.hpp"
#include "cast.hpp"
#include "hires_utc_clock.hpp"
#include <atomic>

#if TT_PROCESSOR == TT_CPU_X64
#include <emmintrin.h>
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include <intrin.h>
#elif TT_OPERATING_SYSTEM == TT_OS_LINUX
#include <x86intrin.h>
#endif
#endif

namespace tt {

class time_stamp_counter {
public:
    constexpr time_stamp_counter() noexcept :
        _counter(0), _id(0) {}

    constexpr time_stamp_counter(int64_t counter, uint32_t id) noexcept :
        _counter(counter), _id(id) {}

    [[nodiscard]] constexpr int64_t counter() const noexcept
    {
        return _counter;
    }

    [[nodiscard]] constexpr uint32_t id() const noexcept
    {
        return _id;
    }

    /** Get the current count from the CPU's time stamp counter.
     * @param memory_order Memory order is one of seq_cst or relaxed.
     * @return The current clock count and cpu-id.
     */
    [[nodiscard]] static time_stamp_counter now(std::memory_order memory_order = std::memory_order::seq_cst) noexcept
    {
        tt_axiom(memory_order != std::memory_order::consume);
        tt_axiom(memory_order != std::memory_order::acq_rel);
        tt_axiom(memory_order != std::memory_order::release);
        tt_axiom(memory_order != std::memory_order::acquire);

        uint64_t counter;
        uint32_t id;

#if TT_PROCESSOR == TT_CPU_X64
        // rdtscp returns both a 64 bit timestamp and a 32 bit opaque cpu-id.
        // The rdtscp instruction includes an implied lfence and mfence instruction
        // before getting the timestamp. An explicit lfence after the rdtscp instruction
        // satisfies the seq_cst memory order.
        unsigned int aux;
        counter = __rdtscp(&aux);
        if (memory_order == std::memory_order::seq_cst) {
            _mm_lfence();
        }

        id = narrow_cast<uint32_t>(aux);
#endif

        return time_stamp_counter{counter, id};
    }

    /** Get a sample.
     * This gets a combination of a TSC and timepoint.
     * Care is taken that the sample was not interrupted by a timeslice.
     */
    [[nodiscard]] static time_stamp_counter get_sample(hires_utc_clock::time_point &tp) noexcept;

    /** Measure the frequence of the time_stamp_counter.
     */
    [[nodiscard] static int64_t measure_frequency() noexcept;

    /** Retrieve the frequency of the time_stamp_counter.
     * This will try to retrieve the frequency of the time_stamp_counter
     * using the cache, the operating system's TSC frequency,
     * or by measuring.
     */
    [[nodiscard]] static int64_t frequency() noexcept;

private:
    int64_t _time_stamp;
    uint32_t _id;

    inline static std::atomic<uint64_t> _frequency;
};

}

