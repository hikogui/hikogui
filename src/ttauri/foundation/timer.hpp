// Copyright 2020 Pokitec
// All rights reserved.


#pragma once

#include "hires_utc_clock.hpp"
#include <mutex>
#include <vector>
#include <functional>
#include <tuple>
#include <thread>

namespace tt {

/** The maintence thread.
 * This thread will execute callbacks at given intervals.
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

private:
    struct callback_entry {
        size_t id;
        duration interval;
        time_point next_wakeup;
        callback_type callback;

        callback_entry(size_t id, duration interval, time_point next_wakeup, callback_type callback) noexcept :
            id(id), interval(interval), next_wakeup(next_wakeup), callback(std::move(callback)) {}
    };

    /** Name of the timer.
     */
    std::string name;

    mutable std::mutex mutex;
    std::thread thread;
    std::vector<callback_entry> callback_list;
    size_t callback_count = 0;

    /** Set to true to ask the thread to exit.
     */
    bool stop_thread;

    /** Find the callbacks that have triggered.
     * This function will also update the wakup times of triggered callbacks.
     *
     * @return List of triggered callbacks, Time to wakeup to trigger on the next callback.
     */
    [[nodiscard]] std::pair<std::vector<callback_type>,timer::time_point> find_triggered_callbacks(
        timer::time_point current_time
    ) noexcept;

    /** The thread procedure.
     */
    void loop() noexcept;

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

public:
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
     * @return An identifier for the callback to be able to remove it.
     */
    [[nodiscard]] size_t add_callback(duration interval, callback_type callback) noexcept;

    /** Remove the callback function.
     */
    void remove_callback(size_t callback_id) noexcept;

};

/** Global maintenance timer.
 */
inline timer maintenance_timer = {"MaintenanceThread"};

}
