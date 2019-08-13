

#include "required.hpp"
#include <atomic>

#pragma once

namespace TTauri {

inline void start_trace(char const *name);
inline void stop_trace();

thread_local trace_span_current_id = 0;
thread_local trace_span_depth = 0;
thread_local trace_span_log_depth = 0;

inline uint64_t trace_span_get_unique_id()
{
    static std::atomic<uint64_t> id = 0;
    return id.fetch_add(1, std::memory_order_relaxed) + 1;
}

struct 

struct trace_span_base {
    using clock = hiperf_utc_clock;

    /*! id of the current trace_span.
     * zero means inactive trace_span
     */
    uint64_t id;

    /*! id of the parent trace_span.
     * zero means inactive trace_span
     */
    uint64_t parent_id;

    /*! Depth of the parent stack.
     */
    uint64_t depth;

    /*! Start timestamp when the trace was started.
     */
    typename clock::timepoint timestamp;

    /*! Duration of the trace-span.
     */
    typename clock::duration duration;

    /*! The default constructor makes the trace inactive.
     */
    trace_span_base() :
        id(0),
        parent_id(0),
        depth(0),
        timestamp()
    {
    }

    /*! The constructor will make the start of a trace.
     *
     * start_trace() should be the only function that will cause this constructor to
     * be executed. start_trace will place this onto current_trace and set this' parent.
     */
    trace_span_base(bool) :
        id(trace_span_get_unique_id()),
        parent_id(trace_span_current_id),
        depth(++trace_span_depth);
        timestamp(clock::now())
    {
    }

    virtual ~trace_span_base() {
        if (id > 0) {
            duration = clock::now() - timestamp;

            // Send the log to the log thread.
            if (is_logging_enabled()) {
                log_param<chars....>(id);
                log_param(id, parent_id);
                log_param(id, timestamp);
                log_param(id, duration);

            }

            // Update global-thread state for the end of this trace-span.
            required_assert(trace_span_depth == depth);
            --trace_span_depth;
            if (trace_span_depth <= trace_span_log_depth) {
                trace_span_log_depth = trace_span_depth;
            }

            trace_span_current_id = parent_id;
        }
    }

    trace_span_base(trace_span_base const &other) = delete;

    trace_span_base(trace_span_base &&other) :
        id(other.id),
        parent_id(other.parent_id),
        timestamp(other.timestamp)
    {
        other.id = 0;    
    }

    trace_span_base &operator=(trace_span_base const &opther) = delete;

    trace_span_base &operator=(trace_span_base &&other) {
        id = other.id;
        parent_id = other.parent_id;
        timestamp = other.timestamp;
        other.id = 0;
        return *this;
    }

    /*! Check if this span needs to be logged.
     * A parent will log when enable_logging() is called on its child.
     * But any childs after of this parent afterwards need not log.
     */
    void is_logging_enabled const {
        return depth <= trace_span_log_depth;
    }

    /*! Enable logging for this span, and all parents below it.
     */
    void enable_logging() const {
        if (depth > trace_span_log_depth) {
            trace_span_log_depth = depth;
        }
    }
};

template<char... chars>
struct trace_span final : public trace_span_base {
    using tag = string_tag<chars...>;
};


template<char... chars>
trace_span<chars...> operator""_trace() {
    return trace_span<chars...>{true};
}

}
