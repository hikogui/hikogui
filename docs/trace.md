# Tracing
Tracing, logging and exception throwing.

## Span
Each thread has a trace stack.

Use `trace::scoped_span` object to start a span which will automatically end when the scope ends.
This span will be added to the top of the stack.

Use `trace::span_start()` and `trace::span_stop()` to have more control of span creation.

A span will have a tag that needs to be checked that it is not used in multiple locations in the source code.
We can possibly use the `__FILENAME__` `__LINE__` to create a compile time tag that is used to create a struct-tag
to use in a template variable. Both a user defined string-tag are logged together with the typeid() of the struct-tag
to use external tools to check if the string-tag was not accidently reused.

The time spend in a span is recorded in a ring buffer and the running average for each power of two is
kept track off. Also the minimum and maxmium is kept up to date. This will allow the logging and monitoring
of these statistics.

## Logging
Any logging with the following macros will be done in the context of the trace-span that is at the top
of the stack:
 * `LOG_DEBUG`
 * `LOG_INFO`
 * `LOG_WARNING`
 * `LOG_EXCEPTION`
 * `LOG_ERROR`
 * `LOG_FATAL`

A span is marked with the highest log level that was used by logging, or by any child-span's logging.

This way the span stack can be completely logged to a file for debugging purpose.

## Exceptions
Exceptions should be thrown using the `TRACED_THROW` macro. This will cause `LOG_EXCEPTION` to be used
to log the exception, and potentially the complete stack of spans to be dumped.

## Counter
Counters are used to count events. Use the `trace::count<>()` function to count events.
All counters start at zero, and are automatically registered for logging at certain intervals.

