// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unfair_mutex.hpp"
#include "subsystem.hpp"
#include "chrono.hpp"
#include <mutex>
#include <vector>
#include <functional>
#include <tuple>
#include <thread>

namespace tt::inline v1 {

/** A timer which will execute callbacks at given intervals.
 */
class timer {
public:
    /** Timer callback.
     * @param current_time The current time of executing this timer.
     * @param last True if this is the last time this timer is called, on emergency stop.
     */
    using callback_type = std::function<void(utc_nanoseconds, bool)>;
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
     * @param immediate When true the callback is immediately called.
     * @return An shared_ptr to retain the callback function, when the shared_ptr is removed then
     *         the callback can no longer be called.
     */
    template<typename Callback>
    [[nodiscard]] std::shared_ptr<callback_type>
    add_callback(std::chrono::nanoseconds interval, Callback callback, bool immediate = false) noexcept
        requires(std::is_invocable_v<Callback, utc_nanoseconds, bool>)
    {
        ttlet current_time = utc_nanoseconds(std::chrono::utc_clock::now());
        auto callback_ptr = std::make_shared<callback_type>(std::forward<Callback>(callback));

        {
            ttlet lock = std::scoped_lock(mutex);

            callback_list.emplace_back(interval, calculate_next_wakeup(current_time, interval), callback_ptr);

            if (ssize(callback_list) == 1) {
                start_with_lock_held();
            }
        }

        if (immediate) {
            (*callback_ptr)(current_time, false);
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
        std::chrono::nanoseconds interval;
        utc_nanoseconds next_wakeup;
        std::weak_ptr<callback_type> callback_ptr;

        callback_entry(
            std::chrono::nanoseconds interval,
            utc_nanoseconds next_wakeup,
            std::shared_ptr<callback_type> const &callback_ptr) noexcept :
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
    std::size_t callback_count = 0;

    /** Find the callbacks that have triggered.
     * This function will also update the wakup times of triggered callbacks.
     *
     * @return List of triggered callbacks, Time to wakeup to trigger on the next callback.
     */
    [[nodiscard]] std::pair<std::vector<callback_ptr_type>, utc_nanoseconds>
    find_triggered_callbacks(utc_nanoseconds current_time) noexcept;

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

    [[nodiscard]] static utc_nanoseconds
    calculate_next_wakeup(utc_nanoseconds current_time, std::chrono::nanoseconds interval) noexcept;

    [[nodiscard]] static timer *subsystem_init() noexcept;
    static void subsystem_deinit() noexcept;
};

} // namespace tt::inline v1
