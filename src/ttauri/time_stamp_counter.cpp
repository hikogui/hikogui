
#include "time_stamp_counter.hpp"

namespace tt {

static time_stamp_counter get_sample(hires_utc_clock &tp) noexcept
{
    constexpr int max_retries = 10;
    int cpu_switch = 0;
    int tsc_stuck = 0;
    int tsc_backward = 0;

    do {
        ttlet id = time_stamp_counter::now().id();

        auto shortest_diff = std::numeric_limits<int64_t>::max();
        time_stamp_counter shortest_tsc;
        hires_utc_clock::time_point shortest_tp;

        // With three samples gathered on the same CPU we should
        // have a TSC/UTC/TSC combination that was run inside a single time-slice.
        for (auto i = 0; i != 3; ++i) {
            ttlet tmp_tsc1 = time_stamp_counter::now();
            ttlet tmp_tp = hires_utc_clock::now();
            ttlet tmp_tsc2 = time_stamp_counter::now();

            if (tmp_tsc1.id() != id || tmp_tsc2.id() != id) {
                ++cpu_switch;
                goto retry;
            }

            ttlet diff = tmp_tsc2.count() - tmp_tsc1.count();
            if (diff == 0) {
                ++tsc_stuck;
                goto retry;
            } else if (diff < 0) {
                ++tsc_backward;
                goto retry;
            }

            if (diff < shortest_diff) {
                shortest_diff = diff;
                shortest_tp = tmp_tp;
                shortest_tsc = tmp_tsc1 + (diff / 2);
            }
        }

        tp = shortest_tp;
        return shortest_tsc;
    retry:
    } while (cpu_switch + tsc_stuck + tsc_backward < max_retries);

    tt_log_fatal("During TSC/UTC sampling, cpu-switch={}, tsc-stuck={}, tsc-backward={}", cpu_switch, tsc_stuck, tsc_backward);
}

[[nodiscard]] int64_t time_stamp_counter::measure_frequency() noexcept
{
    constexpr int max_retries = 10;
    auto cpu_switch = 0;

    do {
        hires_utc_clock::time_point tp1;
        auto tsc1 = get_sample(&tp1);

        hires_utc_clock::time_point tp2;
        auto tsc2 = get_sample(&tp1);

        if (tsc1.id != tsc2.id) {
            ++cpu_switch;
            goto retry;
        }

        auto tsc_diff = tsc2 - tsc1;
        auto tp_diff = tp2 - tp1;
        return 

    retry:
    } while (cpu_switch < max_retries);
    tt_log_fatal("During TSC/UTC frequency measuring, cpu-switch={}", cpu_switch);
}


}

