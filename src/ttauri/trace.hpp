// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "counters.hpp"
#include "datum.hpp"
#include "logger.hpp"
#include "cpu_utc_clock.hpp"
#include "required.hpp"
#include "tagged_map.hpp"
#include "wfree_message_queue.hpp"
#include "fixed_string.hpp"
#include "statistics.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <atomic>
#include <array>
#include <utility>
#include <ostream>
#include <typeinfo>
#include <typeindex>

#pragma once

namespace tt {

constexpr int MAX_NR_TRACES = 1024;


inline std::atomic<int64_t> trace_id = 0;

struct trace_stack_type {
    /*! The trace id of the trace at the top of the thread's stack.
    */
    int64_t top_trace_id = 0;

    /*! The number of currently active traces on this thread.
    */
    int8_t depth = 0;

    /*! Keeps track of the traces that need to record itself into the log.
    */
    int8_t record_depth = 0;

    /*! Push a trace on the trace stack.
    * Traces are in reality already on the thread's actual stack.
    * This function will update a virtual stack of traces.
    * \return parent_id
    */
    inline int64_t push() noexcept {
        ttlet parent_id = top_trace_id;
        top_trace_id = trace_id.fetch_add(1, std::memory_order::relaxed) + 1;
        depth++;
        return parent_id;
    }

    /*! pop a trace from the trace stack.
    * Traces are in reality already on the thread's actual stack.
    * This function will update a virtual stack of traces.
    *
    * \return trace_id, The trace that pops should record itself in the log file.
    */
    inline std::pair<int64_t, bool> pop(int64_t parent_id) noexcept {
        bool is_recording = record_depth > --depth;
        if (is_recording) {
            record_depth = depth;
        }

        ttlet id = top_trace_id;
        top_trace_id = parent_id;
        return {id, is_recording};
    }
};

inline thread_local trace_stack_type trace_stack;

/*! Tell the system to record current trace and all its parents into a log.
*/
void trace_record() noexcept;

template<basic_fixed_string Tag, basic_fixed_string... InfoTags>
struct trace_data {
    /*! id of the parent trace.
    * zero means inactive trace
    */
    int64_t parent_id;

    /*! Start timestamp when the trace was started.
    */
    typename cpu_counter_clock::time_point timestamp;

    tagged_map<sdatum, InfoTags...> info;

    trace_data(typename cpu_counter_clock::time_point timestamp) :
        timestamp(timestamp) {}

    trace_data() = default;
    ~trace_data() = default;
    trace_data(trace_data const &other) = default;
    trace_data &operator=(trace_data const &other) = default;
    trace_data(trace_data &&other) = default;
    trace_data &operator=(trace_data &&other) = default;

    template<basic_fixed_string InfoTag>
    sdatum &get() noexcept {
        return info.template get<InfoTag>();
    }

    template<basic_fixed_string InfoTag>
    sdatum const &get() const noexcept {
        return info.template get<InfoTag>();
    }
};

template<basic_fixed_string Tag, basic_fixed_string... InfoTags>
std::ostream &operator<<(std::ostream &lhs, trace_data<Tag, InfoTags...> const &rhs) {
    auto info_string = std::string{};

    auto counter = 0;
    for (size_t i = 0; i < rhs.info.size(); i++) {
        if (counter++ > 0) {
            info_string += ", ";
        }
        info_string += rhs.info.get_tag(i);
        info_string += "=";
        info_string += static_cast<std::string>(rhs.info[i]);
    }

    lhs << fmt::format("parent={} tag={} start={} {}",
        rhs.parent_id,
        std::type_index(typeid(Tag)).name(),
        format_iso8601(cpu_utc_clock::convert(rhs.timestamp)),
        info_string
    );
    return lhs;
}

/*! Statistics gathered at the destructor of a trace.
 * This is a wait-free structure.
 *  * The trace will acquire by incrementing the count,
 *    and release when writing the same value in the version.
 *  * The reading thread acquires by reading the count,
 *    then release after it has read the version and checked if they hold the same value.
 *  * Since there can be traces in multiple threads, it needs to update the statistics themselves
 *    atomically as well.
 */
class trace_statistics_type {
private:
    std::atomic<int64_t> count = 0;
    std::atomic<typename cpu_counter_clock::rep> duration = 0;
    std::atomic<typename cpu_counter_clock::rep> peak_duration = 0;
    std::atomic<int64_t> version = 0;

    // Variables used by logger.
    int64_t prev_count = 0;
    typename cpu_counter_clock::duration prev_duration = {};

public:
    /*!
     * \return true when this was the first write.
     */
    bool write(cpu_counter_clock::duration const &d) {
        // In the logging thread we can check if count and version are equal
        // to read the statistics.
        ttlet current_count = count.fetch_add(1, std::memory_order::acquire);

        duration.fetch_add(d.count(), std::memory_order::relaxed);

        auto prev_peak = peak_duration.load(std::memory_order::relaxed);
        decltype(prev_peak) new_peak;
        do {
            new_peak = d.count() > prev_peak ? d.count() : prev_peak;
        } while (!peak_duration.compare_exchange_weak(prev_peak, new_peak, std::memory_order::relaxed));

        version.store(current_count + 1, std::memory_order::release);
        
        return current_count == 0;
    }

    struct read_result {
        int64_t count;
        int64_t last_count;

        typename cpu_counter_clock::duration duration;
        typename cpu_counter_clock::duration last_duration;
        typename cpu_counter_clock::duration peak_duration;
    };

    read_result read() {
        read_result r;

        r.peak_duration = {};
        do {
            r.count = count.load(std::memory_order::acquire);

            r.duration = decltype(r.duration){duration.load(std::memory_order::relaxed)};

            auto tmp = peak_duration.exchange(0, std::memory_order::relaxed);
            if (tmp > r.peak_duration.count()) {
                r.peak_duration = decltype(r.duration){tmp};
            }

            std::atomic_thread_fence(std::memory_order::release);
        } while (r.count != version.load(std::memory_order::relaxed));

        r.last_count = r.count - prev_count;
        r.last_duration = r.duration - prev_duration;

        prev_count = r.count;
        prev_duration = r.duration;
        return r;
    }
};

template<basic_fixed_string Tag>
inline trace_statistics_type trace_statistics;

inline wfree_unordered_map<std::string,trace_statistics_type *,MAX_NR_TRACES> trace_statistics_map;


template<basic_fixed_string Tag, basic_fixed_string... InfoTags>
class trace final {
    // If this pointer is not an volatile, clang will optimize it away and replacing it
    // with direct access to the trace_stack variable. This trace_stack variable is in local storage,
    // so a lot of instructions and memory accesses are emitted by the compiler multiple times.
    trace_stack_type * volatile stack;

    trace_data<Tag, InfoTags...> data;

    tt_no_inline static void add_to_map() {
        trace_statistics_map.insert(Tag, &trace_statistics<Tag>);
        statistics_start();
    }

public:
    /*! The constructor will make the start of a trace.
     *
     * start_trace() should be the only function that will cause this constructor to
     * be executed. start_trace will place this onto current_trace and set this' parent.
     */
    trace() :
        stack(&trace_stack), data(cpu_counter_clock::now())
    {
        // We don't need to know our own id, until the destructor is called.
        // Our id will be at the top of the stack.
        data.parent_id = stack->push();
    }

    ~trace() {
        ttlet end_timestamp = cpu_counter_clock::now();

        if(trace_statistics<Tag>.write(end_timestamp - data.timestamp)) {
            [[unlikely]] add_to_map();
        }

        ttlet [id, is_recording] = stack->pop(data.parent_id);

        // Send the log to the log thread.
        if (is_recording) {
            [[unlikely]] tt_log_trace("id={} {}", id, std::move(data));
        }
    }

    trace(trace const &) = delete;
    trace(trace &&) = delete;
    trace &operator=(trace const &) = delete;
    trace &operator=(trace &&) = delete;

    template<basic_fixed_string InfoTag, typename T>
    trace &set(T &&value) {
        data.template get<InfoTag>() = std::forward<T>(value);
        return *this;
    }
};


}
