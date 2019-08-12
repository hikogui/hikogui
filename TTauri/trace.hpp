
#pragma once

namespace TTauri {

inline void start_trace(char const *name);
inline void stop_trace();

struct trace_span {
    using clock = hiperf_utc_clock;

    uint64_t id;
    const char *name;
    std::unique_ptr<trace> parent;
    bool _want_to_log = false;

    typename hiperf_utc_clock::timepoint start_timestamp;
    typename hiperf_utc_clock::timepoint stop_timestamp;

private:
    /*! The constructor will make the start of a trace.
     *
     * start_trace() should be the only function that will cause this constructor to
     * be executed. start_trace will place this onto current_trace and set this' parent.
     */
    trace_span(const char *name) : parent(), name(name) {
        id = get_unique_id();
        start_timestamp = now();
    }

    trace_span() = delete;
    trace_span(trace_span const &other) = delete;
    trace_span(trace_span &&other) = default;
    trace_span &operator=(trace_span const &opther) = delete;
    trace_span &operator=(trace_span &&other) = default;
    ~trace_span() = default;

    /*! Stop a trace.
     *
     */
    stop() {
        stop_timestamp = now();

        if (want_to_log())
    }

    bool want_to_log() {
        if (_want_to_log) {
            return true;
        }

        if (parent && parent->want_to_log()) {
            return true;
        }

    }

    static typename hiperf_utc_clock::timepoint now() {
        return hiperf_utc_clock::now();
    }

    static uint64_t get_unique_id() {
        static std::atomic<uint64_t> id = 1;
        return { i++ };
    }

    friend void start_trace(char const *name);
    friend void stop_trace();
};

inline thread_local auto current_trace = {};

/*! Start a trace_span.
 * XXX static member.
 */
inline void start_trace(char const *name) {
    auto tmp = std::make_unique<trace_span>(name);
    tmp->parent = move(current_trace);
    current_trace = move(tmp);
}

/*! Stop a trace_span.
 */
inline void stop_trace() {
    current_trace->stop();
    current_trace = std::move(current_trace->parent);
}

struct scoped_trace {
    bool holds_trace;

    /*! Private, so only literal strings can use it.
     */
    scoped_trace(char const *name) : holds_trace(true) {
        start_trace(name);
    }

    scoped_trace() = delete;
    scoped_trace(scoped_trace const &other) = delete;
    scoped_trace(scoped_trace &&other) : holds_trace(other.holds_trace) {
        other.holds_trace = false;
    }

    scoped_trace &operator=(scoped_trace const &other) = delete;
    scoped_trace &operator=(scoped_trace &&other)  {
        holds_trace = other.holds_trace;
        other.holds_trace = false;
        return *this;
    }

    ~scoped_trace() {
        if (holds_trace) {
            stop_trace();
        }
    }
};

scoped_trace operator"" _trace(const char *name, size_t name_length)
{
    return scoped_trace(name);
}

}
