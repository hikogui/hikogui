// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


#pragma once

#include "hires_utc_clock.hpp"
#include "unfair_mutex.hpp"
#include "subsystem.hpp"
#include <mutex>
#include <vector>
#include <functional>
#include <tuple>
#include <thread>

namespace tt {

/** A timer which will execute callbacks at given intervals.
 */
class timer {
public:
    using duration = hires_utc_clock::duration;
    using time_point = hires_utc_clock::time_point;

    /** Timer callback.
     * @param current_time The current time of executing this timer.
     * @param last True if this is the last time this timer is called, on emergency stop.
     */
    using callback_type = std::function<void(time_point,bool)>;
    using callback_ptr_type = std::shared_ptr<callback_type>;

    timer(std::string name) noexcept;
    ~timer();

    /** Start the timer thread.
     * Normally it is not needed to call this yourself. If there
     * are no callbacks registered the thread will exit itself.
     */
    void start() noexcept;

    /** Stop the timer thread.
    * Maybe called to emergency stop the timer thread, this will
    * cause all callbacks to be called with last=true.
    */
    void stop() noexcept;

    /** Add a callback function to be executed each interval.
     *
     * The callback will be executed at each interval when:
     *     cpu_utc_clock::now() % interval == 0
     *
     * Since there is only a single thread, please make sure the callback executes quickly.
     *
     * @param interval The interval to execute the callback at.
     * @param callback The callback function.
     * @return An shared_ptr to retain the callback function, when the shared_ptr is removed then
     *         the callback can no longer be called.
     */
    template<typename Callback> requires(std::is_invocable_v<Callback>)
    [[nodiscard]] std::shared_ptr<callback_type> add_callback(duration interval, Callback callback) noexcept
    {
        ttlet lock = std::scoped_lock(mutex);
        ttlet current_time = hires_utc_clock::now();

        auto callback_ptr = std::make_shared<callback_type>(std::forward<Callback>(callback));

        callback_list.emplace_back(
            interval,
            calculate_next_wakeup(current_time, interval),
            callback_ptr
        );

        if (std::ssize(callback_list) == 1) {
            start_with_lock_held();
        }

        return callback_ptr;
    }

    /** Remove the callback function.
     */
    void remove_callback(callback_ptr_type const &callback_ptr) noexcept;

    static timer &global() noexcept
    {
        return *start_subsystem_or_terminate(_global, nullptr, subsystem_init, subsystem_deinit);
    }

private:
    struct callback_entry {
        duration interval;
        time_point next_wakeup;
        std::weak_ptr<callback_type> callback_ptr;

        callback_entry(duration interval, time_point next_wakeup, std::shared_ptr<callback_type> const &callback_ptr) noexcept :
            interval(interval), next_wakeup(next_wakeup), callback_ptr(callback_ptr)
        {
        }
    };

    static inline std::atomic<timer *> _global;

    /** Name of the timer.
     */
    std::string name;

    mutable unfair_mutex mutex;
    std::jthread thread;
    std::vector<callback_entry> callback_list;
    size_t callback_count = 0;

    /** Find the callbacks that have triggered.
     * This function will also update the wakup times of triggered callbacks.
     *
     * @return List of triggered callbacks, Time to wakeup to trigger on the next callback.
     */
    [[nodiscard]] std::pair<std::vector<callback_ptr_type>, timer::time_point>
    find_triggered_callbacks(timer::time_point current_time) noexcept;

    /** The thread procedure.
     */
    void loop(std::stop_token stop_token) noexcept;

    /** Start the timer thread.
     * Normally it is not needed to call this yourself. If there
     * are no callbacks registered the thread will exit itself.
     */
    void start_with_lock_held() noexcept;

    /** Stop the timer thread.
     * Maybe called to emergency stop the timer thread, this will
     * cause all callbacks to be called with last=true.
     */
    void stop_with_lock_held() noexcept;

    [[nodiscard]] static time_point calculate_next_wakeup(time_point current_time, duration interval) noexcept;

    [[nodiscard]] static timer *subsystem_init() noexcept;
    static void subsystem_deinit() noexcept;
};


}
