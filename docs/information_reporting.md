Information reporting
=====================

Exceptions
----------
TTauri will internally use many of the exceptions of the standard library.
The library also includes its own exceptions. These exceptions work the same
as standard library exceptions, except that the constructor works like
std::format for easy construction of the 'what' string.

 - `tt::parse_error`: Files and protocols are under control of users and
   for security reasons it is important to code it defensively with many
   checks. These checks should throw exceptions a `tt::parse_error`.
   For reasons of denial of service errors in a file or protocol
   should never cause the application to crash or terminate.
 - `tt::operation_error`: Thrown when an operation on dynamic types are invalid.
   This can happen when processing a language.
 - `tt::io_error`: Thrown when there is an I/O error.
 - `tt::gui_error`: Exception thrown when errors happen when calling graphic
   APIs like Vulkan.
 - `tt::url_error`: Thrown on invalid URLs.

Assertions
----------

 - `tt_assert()`: Used to check for programming errors, in both debug and
   release builds.
 - `tt_axiom()`: Used to check for programming error in debug builds.
   In release builds the expression is used as a hint to the optimizer.
 - `tt_no_default()`: Used in places that should not be reachable,
   such as default labels of switch statements, or unreachable else
   blocks.
 - `tt_static_no_default()`: Used in unreachable constexpr else blocks.
 - `tt_not_implemented()`: Added when functionality should exist, but
   

Counting
--------
The `tt::increment_counter()` is used to increment a global counter.
The template parameter is a string literal.

This function is designed for low latency and uses an atomic 64 bit counter.
On the first count the counter is registered with the statistics logging
system which logs the counter value each minute.

Tracing
-------
Instantiating a `tt::trace` class starts a trace. A trace will track the amount
if time is spend inside the trace. And extra information may be included with
the trace for debugging purpose.

A lot of care is taken for this function to be efficient and may be used
in a lot of places in the code. It uses thread\_local storage. And it
will be slightly more expensiving than counting.

The average and accumulated time spend inside a trace is logged every minute.

When `tt::trace_record()` is called within a trace all information about
he current trace and any encapsulating traces are logged. `tt::trace_record()`
is implicently called when using `tt_log_error()`.

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
 - `tt_log_debug()`: Logging debug information, which is used while developing
   the application.
 - `tt_log_info()`: Log information messages, which is used while debugging
   an installation of the application.
 - `tt_log_statistics()`: Statistics information logged by the counter and
   tracing system.
 - `tt_log_trace()`: Information logged by the tracing system when _recording_
   a trace.
 - `tt_log_audit()`: Log audit information, for data that must be logged
   for security, business or regulatory reasons.
 - `tt_log_warning()`: Warnings are where something was wrong but the application
   was able to fully recover from it.
 - `tt_log_error()`: An error occurred which causes the application to not be
   fully functional but still able to operate. This will also call `tt::trace_record()`
 - `tt_log_fatal()`: An unrecoverable error has occurred and the application has
   to terminate. This will cause the application to abort.

