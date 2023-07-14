How To Log, Count and Trace
===========================

Logging text
------------

Logging is done using the `hi_log_*()` macros. The argument is a
format string, followed by values to format. The arguments are
near identical to std::format().

```cpp
int i = 5;
float f = 42.0;

hi_log_info("This logs a integer {} and a float {}.", i, f);
```

This example will format and print the text, together with the
cpu-id, thread-id, filename, line-number and the current date
and time when the `hi_log_info()` macro was executed.

There are several log levels:

 | Level      | Macro              | Description                                                                   |
 |:---------- |:------------------ |:---------------------------------------------------------------------------- |
 | fatal      | `hi_log_fatal()`   | Errors that causes harm, including data corruption.                          |
 | error      | `hi_log_error()`   | Errors that causes functionality to not be available.                        |
 | warning    | `hi_log_warning()` | Errors which the application can ignore or solve itself.                     |
 | info       | `hi_log_info()`    | Information possibly useful for a user to fix problems with the application. |
 | debug      | `hi_log_debug()`   | Information possibly useful for a developer.                                 |
 | audit      | `hi_log_audit()`   | Information required for the business or regulatory reasons.                 |
 | statistics | `global_counter<>` | Statistical information for counters and durations.                          |
 | trace      | `trace<>()`        | Debug information about a transaction that was aborted by an exception.      |

### Asynchronous logging

By default logging is done synchronously, meaning that the `hi_log_*()`
macros will block until the log entry is written to console and log files.

Logging synchronously is quite slow, but maybe useful if you do not want
to spawn an extra thread.

By starting the logger, logging will be done asynchronously, messages
are added to a fifo, and a logger-thread will write the messages from the
fifo to the console and log files.

```cpp
// Start the logger-thread and change the log level.
hi::log::start_subsystem(hi::global_state_type::log_level_info);
```

### Wait-free logging

The `hi_log_*()` macros can log wait-free when all the following conditions
are met:

 - The logger-thread must be started to enable asynchronous logging.
 - The logger FIFO must not be full, 1024 messages fit in the FIFO.
 - The format-arguments must have a wait-free copy constructor.
 - The format-arguments are together not more than: 32 bytes.

### Accurate timestamps

To make logging quick, the timestamp is taken from the CPU's
timestamp-counter. The `time_stamp_count` subsystem will calibrate
the timestamp-count of each CPU with the real-time `std::utc_clock`.

After calibration the timestamps in the log are kept close to the
real-time clock, while having an extremely good resolution between
log entries that have been taken on the same-CPU.

```text
Date       Time              CPU Thread     Level Message         Source-filename       Line
---------- ------------------ -- -----      ----- --------------- --------------------- ----
2021-09-28 12:55:15.424435377  7:15112      error This is a test. (gui_window_win32_impl.cpp:536)
```

Wait-free counting
------------------

Instead of logging, you may want to count how often a line of
code is executed. This is a pretty cheap and wait-free operation,
on x86-64 this is done with a single locked-increment/add instruction.

The following line of code increments a counter, the "my counter"
is the name you can give to a counter.

```cpp
++hi::global_counter<"my counter">;
```

The `hi::log::start_subsystem()` function should be called to display these counters
on a per minute interval.

Tracing
-------

Tracing of transactions is done using the `hi::trace<>` type.
A trace records how long the instance of a `hi::trace` stays alive and
logs information when a `hi::trace` is unwound by an exception being thrown.

In the following example the trace object is used to gather statistics on the
"doing calculations" function. How often it is called and; the minimum, maximum
& average duration of the function on each 1 minute interval.

```cpp
void doing_calculations {
    auto t = hi::trace<"doing calculations">{};

    // Do calculations here.
}
```

If you want to log information with the trace, for logging during exceptions, you
can use the following example:

```cpp
void do_transaction(uint64_t user_id, uint64_t transaction_id)
{
    // The '2' is used for the number of slots of information.
    auto t = hi::trace<"do transaction", 2>{};
    t.set("user_id", user_id);
    t.set("transaction_id", transaction_id);

    // Do the transaction here.
}
```
