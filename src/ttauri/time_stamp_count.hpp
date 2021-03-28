// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "os_detect.hpp"
#include "cast.hpp"
#include "int_carry.hpp"
#include "thread.hpp"
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

/** 
 * Since Window's 10 QueryPerformanceCounter() counts at only 10MHz which
 * is too low to measure performance in many cases.
 * 
 * Instead we will use the TSC.
 */
class time_stamp_count {
public:
    constexpr time_stamp_count() noexcept :
        _count(0), _id(0) {}

    constexpr time_stamp_count(uint64_t count, uint32_t id) noexcept :
        _count(count), _id(id) {}

    /** Get the current count from the CPU's time stamp count.
     * @param memory_order Memory order is one of seq_cst or relaxed.
     * @return The current clock count and cpu-id.
     */
    [[nodiscard]] static time_stamp_count now(std::memory_order memory_order = std::memory_order::seq_cst) noexcept
    {
        tt_axiom(memory_order != std::memory_order::consume);
        tt_axiom(memory_order != std::memory_order::acq_rel);
        tt_axiom(memory_order != std::memory_order::release);
        tt_axiom(memory_order != std::memory_order::acquire);

        uint64_t count;
        uint32_t id;

#if TT_PROCESSOR == TT_CPU_X64
        // rdtscp returns both a 64 bit timestamp and a 32 bit opaque cpu-id.
        // The rdtscp instruction includes an implied lfence and mfence instruction
        // before getting the timestamp. An explicit lfence after the rdtscp instruction
        // satisfies the seq_cst memory order.
        unsigned int aux;
        count = __rdtscp(&aux);
        if (memory_order == std::memory_order::seq_cst) {
            _mm_lfence();
        }

        id = narrow_cast<uint32_t>(aux);
#endif

        return time_stamp_count{count, id};
    }

    /** Get the processor index.
     * This is logical processor number that the operating system uses.
     * 
     * @return the processor index, or -1 if the processor index is unknown.
     */
    [[nodiscard]] ssize_t processor() const noexcept;

    /** Get the count since epoch.
     * The epoch is the same as the TSC count's epoch. In most cases the epoch
     * is at system startup time.
     */
    [[nodiscard]] constexpr uint64_t count() const noexcept
    {
        return _count;
    }

    /** Convert to nanoseconds since epoch.
     * The epoch is the same as the TSC count's epoch. In most cases the epoch
     * is at system startup time.
     */
    [[nodiscard]] std::chrono::nanoseconds time_since_epoch() const noexcept
    {
        auto [lo, hi] = wide_mul(_count, _period.load(std::memory_order::relaxed));
        return 1ns * static_cast<int64_t>((hi << 32) | (lo >> 32));
    }

    constexpr time_stamp_count &operator+=(uint64_t rhs) noexcept
    {
        _count += rhs;
        return *this;
    }

    [[nodiscard]] constexpr time_stamp_count operator+(uint64_t rhs) const noexcept
    {
        auto tmp = *this;
        tmp += rhs;
        return tmp;
    }

    /** Measure the frequency of the time_stamp_count.
     * Frequency drift from TSC is 1ppm
     */
    [[nodiscard]] static uint64_t measure_frequency(std::chrono::milliseconds duration) noexcept;

    static void set_frequency(uint64_t frequency) noexcept
    {
        auto period = (uint64_t{1'000'000'000} << 32) / frequency;
        _period.store(period, std::memory_order_relaxed);
    }

    /** Start the time_stamp_count subsystem.
     */
    static void start_subsystem() noexcept;

private:
    uint64_t _count;
    uint32_t _id;

    /** The period in nanoseconds/cycle as Q32.32
     */
    inline static std::atomic<uint64_t> _period;

    /** The number of CPU ids we know of.
     */
    inline static std::atomic<size_t> _num_cpu_ids = 0;

    /** A list of known CPU ids.
     */
    inline static std::array<uint32_t, maximum_num_processors> _cpu_ids;

    /** A list of processor indices that match the _cpu_ids list.
     */
    inline static std::array<size_t, maximum_num_processors> _processor_indices;

    static void populate_cpu_ids() noexcept;
    static void configure_frequency() noexcept;
};

}

