
#include "MaintenanceThread.hpp"

namespace tt {

MaintenanceThread::MaintenanceThread() noexcept
{

}

MaintenanceThread::~MaintenanceThread()
{

}

void MaintenanceThread::start() noexcept
{
}

void MaintenanceThread::stop() noexcept
{
}

void MaintenanceThread::loop() noexcept
{

}

[[nodiscard]] static MaintenanceThread::time_point calculate_next_wakeup(MaintenanceThread::duration interval) noexcept
{
    ttlet current_time = cpu_utc_clock::now();

    ttlet current_time_count = numeric_cast<int64_t>(current_time.time_since_epoch().count());
    ttlet interval_count = numeric_cast<int64_t>(interval.count());

    ttlet intervals_since_epoch = current_time_count / interval_count;

    ttlet next_time_count = (intervals_since_epoch + 1) * interval_count;
    return MaintenanceThread::time_point(MaintenanceThread::duration(next_time_count));
}

[[nodiscard]] size_t MaintenanceThread::add_callback(duration interval, callback_type callback) noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    ttlet callback_id = ++callback_count;

    callback_list.emplace_back(
        callback_id,
        interval,
        calculate_wakeup(interval),
        callback
    );

    if (ssize(callback_list) == 1) {
        start();
    }

    return callback_id;
}


void MaintenanceThread::remove_callback(size_t callback_id) noexcept
{
    ttlet lock = std::scoped_lock(mutex);

    ttlet i = std::find_if(callback_list.cbegin(), callback_list.cend(), [callback_id](ttlet &item) {
        return item.id == callback_id;
    }
    tt_assert(i != callback_list.cend());

    callback_list.erase(i);

    if (ssize(callback_list) == 0) {
        stop();
    }
}

