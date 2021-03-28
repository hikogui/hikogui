
#include "time_stamp_count.hpp"

namespace tt {

[[nodiscard]] ssize_t time_stamp_count::processor() const noexcept
{
    auto cpu_id_ = _mm256_set1_epi32(_id);

    ttlet num_cpu_ids = _num_cpu_ids.load(std::memory_order_acquire);
    tt_axiom(_cpu_ids.size() == _processor_indices.size());
    tt_axiom(num_cpu_ids < _cpu_ids.size());

    for (size_t i = 0; i < num_cpu_ids; i += 8) {
        ttlet row = _mm256_loadu_si256(reinterpret_cast<__m256i const *>(_cpu_ids.data() + i));
        ttlet row_result = _mm256_cmpeq_epi32(row, cpu_id_);
        ttlet row_result_ = _mm256_castsi256_ps(row_result);
        ttlet row_result_mask = _mm256_movemask_ps(row_result_);
        if (static_cast<bool>(row_result_mask)) {
            ttlet cpu_index = i + std::countr_zero(static_cast<unsigned int>(row_result_mask));
            if (cpu_index < num_cpu_ids) {
                return _processor_indices[cpu_index];
            }

            return -1;
        }
    }

    return -1;
}

[[nodiscard]] uint64_t time_stamp_count::measure_frequency(std::chrono::milliseconds sample_duration) noexcept
{
    // Only sample the frequency of one of the TSC clocks.
    auto prev_mask = set_thread_affinity(current_processor());

    time_stamp_count tsc1;
    auto tp1 = hires_utc_clock::now(tsc1);

    std::this_thread::sleep_for(sample_duration);

    time_stamp_count tsc2;
    auto tp2 = hires_utc_clock::now(tsc2);

    // Reset the mask back.
    set_thread_affinity_mask(prev_mask);

    if (tsc1._id != tsc2._id) {
        // This must never happen, as we set the thread affinity to a single CPU
        // if this happens something is seriously wrong.
        tt_log_fatal("CPU Switch detected when measuring the TSC frequency.");
    }

    if (tsc1.count() >= tsc2.count()) {
        // The TSC should only be reset during the very early boot sequence when
        // the CPUs are started and synchronized. It may also happen to a CPU that
        // was hot-swapped while the computer is running, in that case the CPU
        // should not be running applications yet.
        tt_log_fatal("TSC Did not advance during measuring its frequency.");
    }

    if (tp1 >= tp2) {
        // The UTC clock did not advance, maybe a time server changed the clock.
        return 0;
    }

    // Calculate the frequency by dividing the delta-tsc by the duration.
    // We scale both the delta-tsc and duration by 1'000'000'000 before the
    // division. The duration is scaled by 1'000'000'000 by dividing by 1ns.
    ttlet[delta_tsc_lo, delta_tsc_hi] = wide_mul(tsc2.count() - tsc1.count(), uint64_t{1'000'000'000});
    auto duration = narrow_cast<uint64_t>((tp2 - tp1) / 1ns);
    return wide_div(delta_tsc_lo, delta_tsc_hi, duration);
}

void time_stamp_count::populate_cpu_ids() noexcept
{
    // Keep track of the original thread affinity of the main thread.
    auto prev_mask = set_thread_affinity(current_processor());

    // Create a table of cpu_ids.
    size_t next_cpu = 0;
    size_t current_cpu = 0;
    do {
        current_cpu = advance_thread_affinity(next_cpu);

        auto i = _num_cpu_ids.load(std::memory_order::acquire);
        auto tsc = time_stamp_count::now();
        _cpu_ids[i] = tsc._id;
        _processor_indices[i] = current_cpu;
        _num_cpu_ids.store(i + 1, std::memory_order::release);
        tt_log_info("Found CPU {} with TSC:AUX {}.", current_cpu, tsc._id);

    } while (next_cpu > current_cpu);

    // Set the thread affinity back to the original.
    set_thread_affinity_mask(prev_mask);
}

void time_stamp_count::configure_frequency() noexcept
{
    // This function is called from the crt and must therefor be quick as we do not
    // want to keep the user waiting. We are satisfied if the measured frequency is
    // to within 1% accuracy.

    // We take an average over 4 times in case the hires_utc_clock gets reset by a time server.
    uint64_t frequency = 0;
    uint64_t num_samples = 0;
    for (int i = 0; i != 4; ++i) {
        ttlet f = time_stamp_count::measure_frequency(25ms);
        if (f != 0) {
            frequency += f;
            ++num_samples;
        }
    }
    if (num_samples == 0) {
        tt_log_fatal("Unable the measure the frequency of the TSC. The UTC time did not advance.");
    }
    frequency /= num_samples;

    tt_log_info("The measured frequency of the TSC is {} Hz.", frequency);
    time_stamp_count::set_frequency(frequency);
}

[[nodiscard]] void time_stamp_count::start_subsystem() noexcept
{
    configure_frequency();
    populate_cpu_ids();
}

} // namespace tt
