

#include "required.hpp"
#include "counters.hpp"
#include "tagged_map.hpp"
#include "wfree_message_queue.hpp"
#include "hiperf_utc_clock.hpp"
#include "sdatum.hpp"
#include "logger.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <atomic>
#include <array>
#include <utility>
#include <ostream>

#pragma once

namespace TTauri {

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
        let parent_id = top_trace_id;
        top_trace_id = trace_id.fetch_add(1, std::memory_order_relaxed) + 1;
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

        let id = top_trace_id;
        top_trace_id = parent_id;
        return {id, is_recording};
    }
};

inline thread_local trace_stack_type trace_stack;

/*! Tell the system to record current trace and all its parents into a log.
*/
void trace_record() noexcept;

template<string_tag Tag, string_tag... InfoTags>
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

    template<string_tag InfoTag>
    sdatum &get() noexcept {
        return info.template get<InfoTag>();
    }

    template<string_tag InfoTag>
    sdatum const &get() const noexcept {
        return info.get<InfoTag>();
    }
};

template<string_tag Tag, string_tag... InfoTags>
std::ostream &operator<<(std::ostream &lhs, trace_data<Tag, InfoTags...> const &rhs) {
    auto info_string = std::string{};

    auto counter = 0;
    for (size_t i = 0; i < rhs.info.size(); i++) {
        if (counter++ > 0) {
            info_string += ", ";
        }
        info_string += tag_to_string(rhs.info.get_tag(i));
        info_string += "=";
        info_string += static_cast<std::string>(rhs.info[i]);
    }

    lhs << fmt::format("parent={} tag={} start={} {}",
        rhs.parent_id,
        Tag,
        format_iso8601(hiperf_utc_clock::convert(rhs.timestamp)),
        info_string
    );
    return lhs;
}

struct trace_statistics_type {
    std::atomic<int64_t> count = 0;
    cpu_counter_clock::duration duration;
    cpu_counter_clock::duration peak_duration;
    std::atomic<int64_t> version = 0;

    // Variables used by logger.
    int64_t prev_count = 0;
    cpu_counter_clock::duration prev_duration;
    std::atomic<bool> reset = false;
};

template<string_tag Tag>
inline trace_statistics_type trace_statistics;

inline wfree_unordered_map<string_tag,trace_statistics_type *,MAX_NR_TRACES> trace_statistics_map;


template<string_tag Tag, string_tag... InfoTags>
class trace final {
    // If this pointer is not an volatile, clang will optimize it away and replacing it
    // with direct access to the trace_stack variable. This trace_stack variable is in local storage,
    // so a lot of instructions and memory accesses are emited by the compiler multiple times.
    trace_stack_type * volatile stack;

    trace_data<Tag, InfoTags...> data;

    no_inline static void add_to_map() {
        trace_statistics_map.insert(Tag, &trace_statistics<Tag>);
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

    force_inline ~trace() {
        let end_timestamp = cpu_counter_clock::now();
        let duration = end_timestamp - data.timestamp;

        auto &stat = trace_statistics<Tag>;
        let count = stat.count.load(std::memory_order_relaxed);
        let version = count + 1;

        // In the logging thread we can check if count and version are equal
        // to read the statistics.
        stat.count.store(version, std::memory_order_acquire);
        stat.duration += duration;

        let reset = stat.reset.load(std::memory_order_relaxed);
        if (reset || duration > stat.peak_duration) {
            stat.peak_duration = duration;
            stat.reset.store(false, std::memory_order_relaxed);
        }

        stat.version.store(version, std::memory_order_release);

        if (ttauri_unlikely(count == 0)) {
            add_to_map();
        }

        let [id, is_recording] = stack->pop(data.parent_id);

        // Send the log to the log thread.
        if (ttauri_unlikely(is_recording)) {
            logger.log<log_level::Trace>(end_timestamp, "id={} {}", id, std::move(data));
        }
    }

    trace(trace const &) = delete;
    trace(trace &&) = delete;
    trace &operator=(trace const &) = delete;
    trace &operator=(trace &&) = delete;

    template<string_tag InfoTag, typename T>
    trace &set(T &&value) {
        data.template get<InfoTag>() = std::forward<T>(value);
        return *this;
    }
};


}
