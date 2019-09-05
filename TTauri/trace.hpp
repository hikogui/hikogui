

#include "required.hpp"
#include "counters.hpp"
#include "small_map.hpp"
#include "wfree_mpsc_message_queue.hpp"
#include "hiperf_utc_clock.hpp"
#include "datum.hpp"
#include "logger.hpp"
#include <atomic>
#include <array>
#include <utility>
#include <ostream>

#pragma once

namespace TTauri {


inline std::atomic<int64_t> trace_id = 0;

struct trace_stack_type {
    /*! The number of currently active traces on this thread.
    */
    int depth = 0;

    /*! Keeps track of the traces that need to record itself into the log.
    */
    int record_depth = 0;

    /*! The trace id of the trace at the top of the thread's stack.
    */
    int64_t top_trace_id = 0;

    /*! Push a trace on the trace stack.
    * Traces are in reality already on the thread's actual stack.
    * This function will update a virtual stack of traces.
    */
    inline std::pair<int64_t,int64_t> push() noexcept {
        let id = trace_id.fetch_add(1, std::memory_order_relaxed) + 1;
        let parent_id = top_trace_id;
        top_trace_id = id;
        depth++;
        return {id, parent_id};
    }

    /*! Check if the current trace is being recorded into a log.
    */
    bool is_recording() const noexcept {
        return depth <= record_depth;
    }

    /*! pop a trace from the trace stack.
    * Traces are in reality already on the thread's actual stack.
    * This function will update a virtual stack of traces.
    *
    * \return The trace that pops should record itself in the log file.
    */
    inline bool pop(int64_t parent_id) noexcept {
        bool is_recording = record_depth > --depth;
        if (is_recording) {
            record_depth = depth;
        }

        top_trace_id = parent_id;
        return is_recording;
    }
};

inline thread_local trace_stack_type trace_stack;

/*! Tell the system to record current trace and all its parents into a log.
*/
void trace_record() noexcept;

struct trace_data {
    string_tag tag = 0;

    /*! id of the current trace.
    * zero means inactive trace
    */
    int64_t id = 0;

    /*! id of the parent trace.
    * zero means inactive trace
    */
    int64_t parent_id = 0;

    /*! Start timestamp when the trace was started.
    */
    typename cpu_counter_clock::time_point timestamp;

    /*! Information added to a trace during its lifetime.
    */
    small_map<string_tag,datum,8> trace_info;

    trace_data(string_tag tag, typename cpu_counter_clock::time_point timestamp) :
        tag(tag), timestamp(timestamp) {}

    trace_data() = default;
    ~trace_data() = default;
    trace_data(trace_data const &other) = default;
    trace_data &operator=(trace_data const &other) = default;
    trace_data(trace_data &&other) = default;
    trace_data &operator=(trace_data &&other) = default;

    template<string_tag InfoTag, typename T>
    trace_data &set(T &&value) {
        trace_info.push(InfoTag, std::forward<T>(value));
        return *this;
    }

    template<string_tag InfoTag>
    std::optional<datum> get() {
        return trace_info.get(InfoTag);
    }

    template<string_tag InfoTag>
    datum get(datum default_value) {
        return trace_info.get(InfoTag, default_value);
    }
};

std::ostream &operator<<(std::ostream &lhs, trace_data const &rhs);

template<string_tag Tag>
class trace final {
    trace_stack_type *stack;
    trace_data data;
    char const *source_file = nullptr;
    int source_line = 0;

public:
    /*! The constructor will make the start of a trace.
     *
     * start_trace() should be the only function that will cause this constructor to
     * be executed. start_trace will place this onto current_trace and set this' parent.
     */
    trace(char const *source_file, int source_line) :
        stack(&trace_stack), data(Tag, cpu_counter_clock::now()), source_file(source_file), source_line(source_line)
    {
        std::tie(data.id, data.parent_id) = stack->push();

        increment_counter<Tag>();
    }

    ~trace() {
        let is_recording = stack->pop(data.parent_id);

        // Send the log to the log thread.
        if (ttauri_unlikely(is_recording)) {
            logger.log<log_level::Trace>(source_file, source_line, "{}", std::move(data));
        }
    }

    trace(trace const &) = delete;
    trace(trace &&) = delete;
    trace &operator=(trace const &) = delete;
    trace &operator=(trace &&) = delete;

    template<string_tag InfoTag, typename T>
    trace &set(T &&value) {
        data.set<InfoTag>(std::forward<T>(value));
        return *this;
    }
};


#define TTAURI_TRACE(Tag) ::TTauri::trace<Tag>(__FILE__, __LINE__);

}
