
#include "hires_utc_clock.hpp"
#include <mutex>
#include <vector>
#include <functional>

namespace tt {

/** The maintence thread.
 * This thread will execute callbacks at given intervals.
 */
class MaintenanceThread {
public:
    using duration = hires_utc_clock::duration;
    using time_point = hires_utc_clock::time_point;
    using callback_type = std::function<void(time_point)>;

private:
    struct callback_entry {
        size_t id;
        duration interval;
        time_point next_wakeup;
        callback_type callback;
    };

    mutable std::recursive_mutex mutex;
    std::thread thread;
    std::vector<callback_entry> callback_list;
    size_t callback_count = 0;

    /** The thread procedure.
     */
    void loop() noexcept;

    void start() noexcept;

    void stop() noexcept;

public:
    MaintenanceThread() noexcept;
    ~MaintenanceThread();


    /** Add a callback function to be executed each interval.
     *
     * The callback will be executed at each interval when:
     *     cpu_utc_clock::now() % interval == 0
     *
     * Since there is only a single thread, please make sure the callback executes quickly.
     *
     * @param interval The interval to execute the callback at.
     */
    [[nodiscard]] size_t add_callback(duration interval, callback_type callback) noexcept;

    /** Remove the callback function.
     */
    void remove_callback(size_t callback_id) noexcept;

};

inline MaintenanceThread maintananceThread;


}
