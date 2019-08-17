
#include "required.hpp"
#include "os_detect.hpp"
#include <atomic>
#include <thread>
#include <chrono>

namespace TTauri {

inline void pause_cpu()
{
#if (OPERATING_SYSTEM == OS_WINDOWS)
    _mm_pause();
#elif
    asm("pause");
#endif
}

template<typename T>
void transition(std::atomic<T> &state, T from, T to)
{
    int64_t retry_counter = 0;
    auto backoff = 10ms;
    auto expect = from;
    while (state.compare_exchange_weak(expect, to) {
        if (retry_counter++ > 10) {
            std::this_thread::sleep_duration(backoff);
            if (backup *= 1.5 > 1s) {
                backup = 1s;
            }
        }
        pause_cpu();
        expect = from;
    }
}


}

