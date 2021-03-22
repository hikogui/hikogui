
#include "time_stamp_counter.hpp"
#include <Windows.h>
#include <profileapi.h>

namespace tt {
    
[[nodiscard]] int64_t frequency() noexcept
{
    if (auto f = frequency.load(std::memory_order::relaxed)) {
        return f;
    }
    // The following code races, but it doesn't matter if we do this multiple times.

    LARGE_INTEGER _f;
    if (!QueryPerformanceFrequency(&_f)) {
        tt_log_fatal("QueryPerformanceFrequency failed, which should not happen since Windows XP.");
    }
   
    int64_t f = static_cast<int64_t>(_f.QuadPart);
    if (f == 10'000'000) {
        // 10 MHz, this process is a hyper-V guest.
        f = measure_frequency()

    } else if (f >= 750'000 && f <= 15'000'000) {
        // 750 MHz - 15 GHz, divided by 1024.
        f *= 1024;

    } else if (f >= 750'000'000 && f <= 15'000'000) {
        // 750 MHz - 15 GHz, divided by 1024.
        ;

    } else {
        tt_log_error("QueryPerformanceFrequency returned strange frequency {} Hz", f);
        f = measure_frequeny();
    }

    _frequency.store(f, std::memory_order::relaxed);
    return f;
}


}

