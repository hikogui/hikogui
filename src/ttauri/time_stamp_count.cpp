
#include "time_stamp_count.hpp"

namespace tt {

[[nodiscard]] time_stamp_count time_stamp_count::get_sample(hires_utc_clock::time_point &tp) noexcept
{
    constexpr int max_retries = 10;
    int cpu_switch = 0;
    int tsc_stuck = 0;
    int tsc_backward = 0;

    do {
        ttlet id = time_stamp_count::now().id();

        auto shortest_diff = std::numeric_limits<uint64_t>::max();
        time_stamp_count shortest_tsc;
        hires_utc_clock::time_point shortest_tp;

        // With three samples gathered on the same CPU we should
        // have a TSC/UTC/TSC combination that was run inside a single time-slice.
        for (auto i = 0; i != 3; ++i) {
            ttlet tmp_tsc1 = time_stamp_count::now();
            ttlet tmp_tp = hires_utc_clock::now();
            ttlet tmp_tsc2 = time_stamp_count::now();

            if (tmp_tsc1.id() != id || tmp_tsc2.id() != id) {
                ++cpu_switch;
                goto retry;
            }

            if (tmp_tsc1.count() == tmp_tsc2.count()) {
                ++tsc_stuck;
                goto retry;
            } else if (tmp_tsc1.count() > tmp_tsc2.count()) {
                ++tsc_backward;
                goto retry;
            }

            ttlet diff = tmp_tsc2.count() - tmp_tsc1.count();

            if (diff < shortest_diff) {
                shortest_diff = diff;
                shortest_tp = tmp_tp;
                shortest_tsc = {tmp_tsc1.count() + (diff / 2), tmp_tsc1.id()};
            }
        }

        tp = shortest_tp;
        return shortest_tsc;
retry:;
    } while (cpu_switch + tsc_stuck + tsc_backward < max_retries);

    tt_log_fatal("During TSC/UTC sampling, cpu-switch={}, tsc-stuck={}, tsc-backward={}", cpu_switch, tsc_stuck, tsc_backward);
}

[[nodiscard]] uint64_t time_stamp_count::measure_frequency(hires_utc_clock::duration duration) noexcept
{
    auto prev_mask = set_thread_affinity(current_processor());

    hires_utc_clock::time_point tp1;
    auto tsc1 = get_sample(tp1);

    std::this_thread::sleep_for(duration);

    hires_utc_clock::time_point tp2;
    auto tsc2 = get_sample(tp2);

    set_thread_affinity_mask(prev_mask);

    tt_axiom(tsc1.id() == tsc2.id());

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
