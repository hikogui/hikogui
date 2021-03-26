
#include "time_stamp_count.hpp"

namespace tt {

[[nodiscard]] uint64_t time_stamp_count::measure_frequency(std::chrono::milliseconds duration) noexcept
{
    auto prev_mask = set_thread_affinity(current_processor());

    time_stamp_count tsc1;
    auto tp1 = hires_utc_clock::now(tsc1);

    std::this_thread::sleep_for(duration);

    time_stamp_count tsc2;
    auto tp2 = hires_utc_clock::now(tsc2);

    set_thread_affinity_mask(prev_mask);

    if (tsc1.id() != tsc2.id()) {
        tt_log_fatal("CPU Switch detected when measuring the TSC frequency.");
    }

    if (tsc1.count() >= tsc2.count()) {
        tt_log_fatal("TSC Did not go forward during measuring its frequency.");
    }

    auto tsc_diff = tsc2.count() - tsc1.count();
    auto tp_diff = tp2 - tp1;
    return (tsc_diff * 10'000'000) / (tp_diff / 100ns);
}

[[nodiscard]] void time_stamp_count::start() noexcept
{
    // Measuring for one second is enough to get a 4GHz TSC frequency accurate to within 10kHz.
    // 
    // This function is called from the crt, this should be very fast an accuracy should be within 1%.
    // 
    // A std::this_thread::sleep_for(1ms) seems to trigger a wait for a much longer time than
    // just using std::this_thread::yield() presumably yield() continues directly when the CPU is free, while sleep_for()
    // causes a wait for at least a full time-slice.
    ttlet frequency = time_stamp_count::measure_frequency(1ms);
    tt_log_info("The frequency according to the operating system is {} Hz.", frequency);
    
    // since the frequency is only accurate to 1 ppm, that means we only need 20 bits
    // of accuracy in the (ns / cycle) period. Use a f32.32 format for the period.

    auto period = (uint64_t{1'000'000'000} << 32) / frequency;

    _period.store(period, std::memory_order_release);
}

} // namespace tt
