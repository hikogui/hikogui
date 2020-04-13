
#include "TTauri/Foundation/cpu_utc_clock.hpp"

#pragma once

namespace TTauri {

/** Information on when to trigger.
 */
class Trigger {
    struct trigger_t {
        cpu_utc_clock::time_point time_point;
        int level;
    };

    Trigger *parent;

    /// When to wake the widget/window.
    std::atomic<trigger_t> event;

public:
    Trigger(Trigger *parent=nullptr) noexcept :
        parent(parent), event(cpu_utc_clock::time_point::min(), 1) {}

    Trigger(Trigger const &) noexcept = delete;
    Trigger(Trigger &&) noexcept = delete;
    Trigger &operator=(Trigger const &) noexcept = delete;
    Trigger &operator=(Trigger &&) noexcept = delete;
    ~Trigger() = default;

    /** Add a time_point to the trigger.
     * Both time_point and level are atomically updated, however
     * the minimum time_point and maximum level is recorded independently.
     * determined.
     *
     * Level should be larger than zero, zero means idle when using the
     * `check()` method.
     *
     * @param time_point The next moment to trigger.
     * @param level The level of the trigger.
     * @return this instance.
     */
    Trigger &add(cpu_utc_clock::time_point time_point, int level) noexcept {
        ttauri_assume(level > 0);

        auto old_event = event.load();
        do {
            auto new_event = trigger_t{
                std::min(old_event.time_point, time_point),
                std::max(old_event.level, level)
            };

        } while (!event.compare_exchange_strong(&old_event, new_event));

        if (parent) {
            parent->request(time_point, level);
        }
    }

    /** Retrieve the trigger level at the current time.
     * This function will destructivly and atomically read the trigger level.
     *
     * @param current_time The current time.
     * @return The highest level of a set trigger, or zero when it is not triggered.
     */
    int check(cpu_utc_clock::time_point current_time) noexcept {
        auto old_event = event.load();
        do {
            if (old_event.time_point > current_time) {
                return 0;
            }

            // Set next trigger to the far future.
            auto new_event = trigger_t{cpu_utc_clock::time_point::max(), 0};
        } while (!event.compare_exchange_strong(&old_event, new_event));

        return old_event.level;
    }

    /** Set to imidiatly trigger at level 1.
     */
    Trigger &operator++() noexcept {
        return add(cpu_utc_clock::time_point::min(), 1);
    }

    /** Set to trigger at a specified time at level 1.
     */
    Trigger &operator+=(cpu_utc_clock::time_point time_point) noexcept {
        return add(time_point, 1);
    }

    /** Set to imidiatly trigger with a specified level.
     */
    template<typename T, std::enable_if_t<std::is_arithmatic_v<T>,int>=0>
    Trigger &operator+=(T level) noexcept {
        return add(cpu_utc_clock::time_point::min(), static_cast<int>(level));
    }

};

}
