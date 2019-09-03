

#include "required.hpp"
#include "counters.hpp"
#include "small_map.hpp"
#include "wfree_mpsc_message_queue.hpp"
#include "hiperf_utc_clock.hpp"
#include "datum.hpp"
#include <atomic>
#include <array>
#include <utility>

#pragma once

namespace TTauri {

using trace_id = int64_t;

struct scoped_span_base;

struct trace_message {
    string_tag tag;
    trace_id id;
    trace_id parent_id;
    hiperf_utc_clock::timepoint timestamp;
    hiperf_utc_clock::duration duration;
    small_map<string_tag,datum,16> trace_info;

    trace_message(scoped_span_base const &span);
};

class trace {
private:
    thread_local int depth = 0;
    thread_local int record_depth = 0;
    static std::atomic<trace_id> id = 0;

    wfree_mpsc_message_queue<trace_message,2048> messages;
public:
    thread_local trace_id current_id = 0;

    static trace_id get_unique_id() noexcept {
        return id.fetch_add(1, std::memory_order_relaxed) + 1;
    }

    static std::pair<trace_id,trace_id> push() noexcept {
        let id = get_unique_id();
        let parent_id = current_id;
        current_id = id;
        depth++;
        return {id, parent_id};
    }

    static void pop(trace_id parent_id) noexcept {
        required_assert(depth > 0);
        if (record_depth > --depth) {
            record_depth = depth;
        }

        current_id = parent_id;
    }

    /*! Check if the current trace is being recorded into a log.
     */
    static bool is_recording() noexcept {
        return depth <= record_depth;
    }

    /*! Tell the system to record current trace and all its parents into a log.
     */
    static void record() noexcept {
        if (!is_recording()) {
            record_depth = depth;
        }
    }

    /*! Record a message into the log.
     */
    static void record_message(trace_messsage const &message) {
        messages.push_back(message);
    }
};

struct scoped_span_base {
    using clock = hiperf_utc_clock;

    /*! id of the current trace_span.
     * zero means inactive trace_span
     */
    trace_id id;

    /*! id of the parent trace_span.
     * zero means inactive trace_span
     */
    trace_id parent_id;

    /*! Start timestamp when the trace was started.
     */
    typename clock::timepoint timestamp;

    /*! Duration of the trace-span.
     */
    typename clock::duration duration;

    /*! Information added to a trace during its lifetime.
     * int64_t as values for wait-free value transfer.
     */
    small_map<string_tag,int64_t,16> trace_info;

    /*! The constructor will make the start of a trace.
     *
     * start_trace() should be the only function that will cause this constructor to
     * be executed. start_trace will place this onto current_trace and set this' parent.
     */
    scoped_span_base() :
        timestamp(clock::now())
    {
        std::tie(id, parent_id) = trace::push();
    }

    virtual ~scoped_span_base() {
        trace::pop(parent_id);
    }

    scoped_span_base(scoped_span_base const &) = delete;
    scoped_span_base(scoped_span_base &&) = delete;
    scoped_span_base &operator=(scoped_span_base const &) = delete;
    scoped_span_base &operator=(scoped_span_base &&) = delete;

    template<string_tag tag>
    bool insert(int64_t value) {
        trace_info.push(tag, value);
    }

    template<string_tag tag>
    std::optional<int64_t> get() {
        return trace_info.get(tag);
    }

    template<string_tag tag>
    int64_t get(int64_t default_value) {
        return trace_info.get(tag, default_value);
    }

    virtual string_tag tag() const noexcept = 0;
};

template<string_tag _TAG>
struct scoped_span final : public scoped_span_base {
    static constexpr string_tag TAG = _TAG;

    scoped_span() {
        increment_counter<TAG>();
    }

    ~scoped_span() {
        if (id > 0) {
            duration = clock::now() - timestamp;

            // Send the log to the log thread.
            if (ttauri_unlikely(trace::is_recording())) {
                let message = trace_message(TAG, *this);
                trace::log_message(message);
            }
        }
    }

    string_tag tag() const noexcept override { return TAG; }
};

trace_message::trace_message(string_tag tag, scoped_span_base const &span) :
    tag(tag),
    id(trace.id),
    parent_id(trace.parent_id),
    timestamp(trace.timestamp),
    duration(trace.duration),
    trace_info(trace.trace_info)
{
}


}
