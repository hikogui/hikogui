

#include "required.hpp"
#include "counters.hpp"
#include "tagged_map.hpp"
#include "wfree_message_queue.hpp"
#include "hiperf_utc_clock.hpp"
#include "sdatum.hpp"
#include "logger.hpp"
#include <fmt/format.h>
#include <atomic>
#include <array>
#include <utility>
#include <ostream>

#pragma once

namespace TTauri {

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

template<string_tag... InfoTags>
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

template<string_tag... InfoTags>
std::ostream &operator<<(std::ostream &lhs, trace_data<InfoTags...> const &rhs) {
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

    lhs << fmt::format("parent={} start={} {}",
        rhs.parent_id,
        format_iso8601(hiperf_utc_clock::convert(rhs.timestamp)),
        info_string
    );
    return lhs;
}

template<string_tag Tag, string_tag... InfoTags>
class trace final {
    trace_stack_type *stack;
    trace_data<Tag, InfoTags...> data;

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

        increment_counter<Tag>();
    }

    force_inline ~trace() {
        let [id, is_recording] = stack->pop(data.parent_id);

        // Send the log to the log thread.
        if (ttauri_unlikely(is_recording)) {
            logger.log<log_level::Trace>("tag={}, id={} {}", Tag, id, std::move(data));
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
