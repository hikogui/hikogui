Information reporting
=====================

Exceptions
----------

HikoGUI uses many of the exceptions provided by the standard library.
But the library also includes its own exceptions. These exceptions work the same
as standard library exceptions, except that the constructor works like
std::format for easy construction of the 'what' string.

 - `hi::parse_error`: Files and protocols are under control of users and
   for security reasons it is important to code it defensively with many
   checks. These checks should throw exceptions a `hi::parse_error`.
   For reasons of denial of service errors in a file or protocol
   should never cause the application to crash or terminate.
 - `hi::operation_error`: Thrown when an operation on dynamic types are invalid.
   This can happen when processing a language.
 - `hi::io_error`: Thrown when there is an I/O error.
 - `hi::gui_error`: Exception thrown when errors happen when calling graphic
   APIs like Vulkan.
 - `hi::url_error`: Thrown on invalid URLs.

Assertions
----------

 - `hi_assert()`: Used to check for programming errors, in both debug and
   release builds.
 - `hi_axiom()`: Used to check for programming error in debug builds.
   In release builds the expression is used as a hint to the optimizer.
 - `hi_no_default()`: Used in places that should not be reachable,
   such as default labels of switch statements, or unreachable else
   blocks.
 - `hi_static_no_default()`: Used in unreachable constexpr else blocks.
 - `hi_not_implemented()`: Added when functionality should exist, but is not implemented, yet.

Counting
--------

The `hi::increment_counter()` is used to increment a global counter.
The template parameter is a string literal.

This function is designed for low latency and uses an atomic 64 bit counter.
On the first count the counter is registered with the statistics logging
system which logs the counter value each minute.

Tracing
-------

Instantiating a `hi::trace` class starts a trace. A trace will track the amount
if time is spend inside the trace. Extra information may be included with
the trace for debugging purpose.

A lot of care is taken for this function to be efficient and may be used
in a lot of places in the code. It uses thread_local storage. And it
will be slightly more expensive than counting.

The average and accumulated time spend inside a trace is logged every minute.

When `hi::trace_record()` is called within a trace all information about
the current trace and any encapsulating traces are logged. `hi::trace_record()`
is implicitly called when using `hi_log_error()`.

Logging
-------

The logging system is for logs textual messages to a console or a file.
It is the main system of high level debugging.

A lot of care was taken to be efficient, but it is more expensive than
either counting or tracing. The logger functions work like std::format
however the actual formatting is delayed and offloaded to the logger thread.

Logging is wait-free, unless:

 - the ring buffer is full which causes the logging to block.
 - objects passed as argument allocate on copy.
 - All the objects together occupies more bytes than the message
   in the ring buffer 224 bytes, which causes an allocation.

The following log functions are available:

 - `hi_log_debug()`: Logging debug information, which is used while developing
   the application.
 - `hi_log_info()`: Log information messages, which is used while debugging
   an installation of the application.
 - `hi_log_statistics()`: Statistics information logged by the counter and
   tracing system.
 - `hi_log_trace()`: Information logged by the tracing system when _recording_
   a trace.
 - `hi_log_audit()`: Log audit information, for data that must be logged
   for security, business or regulatory reasons.
 - `hi_log_warning()`: Warnings are where something was wrong but the application
   was able to fully recover from it.
 - `hi_log_error()`: An error occurred which causes the application to not be
   fully functional but still able to operate. This will also call `hi::trace_record()`.
 - `hi_log_fatal()`: An unrecoverable error has occurred and the application has
   to terminate. This will cause the application to abort.
