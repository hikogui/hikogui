// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "time_stamp_count.hpp"
#include "time_stamp_utc.hpp"
#include "log.hpp"
#include <emmintrin.h>
#include <array>
#include <cstdint>

namespace hi::inline v1 {

[[nodiscard]] ssize_t time_stamp_count::cpu_id_fallback() const noexcept
{
    auto aux_value_ = _mm_set1_epi32(_aux);

    hilet num_aux_values = _num_aux_values.load(std::memory_order_acquire);
    hi_assert(_aux_values.size() == _cpu_ids.size());
    hi_assert_bounds(num_aux_values, _aux_values);

    for (std::size_t i = 0; i < num_aux_values; i += 4) {
        hilet row = _mm_loadu_si128(reinterpret_cast<__m128i const *>(_aux_values.data() + i));
        hilet row_result = _mm_cmpeq_epi32(row, aux_value_);
        hilet row_result_ = _mm_castsi128_ps(row_result);
        hilet row_result_mask = _mm_movemask_ps(row_result_);
        if (to_bool(row_result_mask)) {
            hilet j = i + std::countr_zero(static_cast<unsigned int>(row_result_mask));
            if (j < num_aux_values) {
                return _cpu_ids[j];
            }

            return -1;
        }
    }

    return -1;
}

[[nodiscard]] uint64_t time_stamp_count::measure_frequency(std::chrono::milliseconds sample_duration) noexcept
{
    using namespace std::chrono_literals;

    // Only sample the frequency of one of the TSC clocks.
    auto prev_mask = set_thread_affinity(current_cpu_id());

    time_stamp_count tsc1;
    auto tp1 = time_stamp_utc::now(tsc1);

    std::this_thread::sleep_for(sample_duration);

    time_stamp_count tsc2;
    auto tp2 = time_stamp_utc::now(tsc2);

    // Reset the mask back.
    set_thread_affinity_mask(prev_mask);

    if (tsc1._aux != tsc2._aux) {
        // This must never happen, as we set the thread affinity to a single CPU
        // if this happens something is seriously wrong.
        hi_log_fatal("CPU Switch detected when measuring the TSC frequency.");
    }

    if (tsc1.count() >= tsc2.count()) {
        // The TSC should only be reset during the very early boot sequence when
        // the CPUs are started and synchronized. It may also happen to a CPU that
        // was hot-swapped while the computer is running, in that case the CPU
        // should not be running applications yet.
        hi_log_fatal("TSC Did not advance during measuring its frequency.");
    }

    if (tp1 >= tp2) {
        // The UTC clock did not advance, maybe a time server changed the clock.
        return 0;
    }

    // Calculate the frequency by dividing the delta-tsc by the duration.
    // We scale both the delta-tsc and duration by 1'000'000'000 before the
    // division. The duration is scaled by 1'000'000'000 by dividing by 1ns.
    hilet[delta_tsc_lo, delta_tsc_hi] = mul_carry(tsc2.count() - tsc1.count(), uint64_t{1'000'000'000});
    auto duration = narrow_cast<uint64_t>((tp2 - tp1) / 1ns);
    return wide_div(delta_tsc_lo, delta_tsc_hi, duration);
}

void time_stamp_count::populate_aux_values() noexcept
{
    // Keep track of the original thread affinity of the main thread.
    auto prev_mask = set_thread_affinity(current_cpu_id());

    // Create a table of cpu_ids.
    std::size_t next_cpu = 0;
    std::size_t current_cpu = 0;
    bool aux_is_cpu_id = true;
    do {
        current_cpu = advance_thread_affinity(next_cpu);

        auto i = _num_aux_values.load(std::memory_order::acquire);
        auto tsc = time_stamp_count::now();
        _aux_values[i] = tsc._aux;
        _cpu_ids[i] = current_cpu;
        _num_aux_values.store(i + 1, std::memory_order::release);
        hi_log_info("Found CPU {} with TSC:AUX {}.", current_cpu, tsc._aux);

        if ((tsc._aux & 0xfff) != current_cpu) {
            aux_is_cpu_id = false;
        }

    } while (next_cpu > current_cpu);

    _aux_is_cpu_id.store(aux_is_cpu_id, std::memory_order_relaxed);
    if (aux_is_cpu_id) {
        hi_log_info("Use fast time_stamp_count.cpu_id() implementation.");
    }

    // Set the thread affinity back to the original.
    set_thread_affinity_mask(prev_mask);
}

void time_stamp_count::configure_frequency() noexcept
{
    using namespace std::chrono_literals;

    // This function is called from the crt and must therefor be quick as we do not
    // want to keep the user waiting. We are satisfied if the measured frequency is
    // to within 1% accuracy.

    // We take an average over 4 times in case the hires_utc_clock gets reset by a time server.
    uint64_t frequency = 0;
    uint64_t num_samples = 0;
    for (int i = 0; i != 4; ++i) {
        hilet f = time_stamp_count::measure_frequency(25ms);
        if (f != 0) {
            frequency += f;
            ++num_samples;
        }
    }
    if (num_samples == 0) {
        hi_log_fatal("Unable the measure the frequency of the TSC. The UTC time did not advance.");
    }
    frequency /= num_samples;

    hi_log_info("The measured frequency of the TSC is {} Hz.", frequency);
    time_stamp_count::set_frequency(frequency);
}

void time_stamp_count::start_subsystem() noexcept
{
    configure_frequency();
    populate_aux_values();
}

} // namespace hi::inline v1
