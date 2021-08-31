How To Log, Count and Trace
===========================

Logging text
------------

Logging is done using the `tt_log_*()` macros. The argument is a
format string, followed by values to format. The arguments are
near identical to std::format().

```cpp
int i = 5;
float f = 42.0;

tt_log_info("This logs a integer {} and a float {}.", i, f);
```

This example will format and print the text, together with the
cpu-id, thread-id, filename, line-number and the current date
and time when the `tt_log_info()` macro was executed.

There are several log levels:

 | Level      | Macro              | Desription                                                                   |
 |:---------- |:------------------ |:---------------------------------------------------------------------------- |
 | fatal      | `tt_log_fatal()`   | Errors that causes harm, including data corruption.                          |
 | error      | `tt_log_error()`   | Errors that causes functionality to not be available.                        |
 | warning    | `tt_log_warning()` | Errors which the application can ignore or solve itself.                     |
 | info       | `tt_log_info()`    | Information possibly useful for a user to fix problems with the application. |
 | debug      | `tt_log_debug()`   | Information possibly useful for a developer.                                 |
 | audit      | `tt_log_audit()`   | Information required for the business or regulatory reasons.                 |
 | statistics | `global_counter<>` | Statistical information for counters and durations.                          |
 | trace      | `trace<>()`        | Debug information about a transaction that was aborted by an exception.      |


### Asynchronous logging
By default logging is done synchronously, meaning that the `tt_log_*()`
macros will block until the log entry is written to console and log files.

Logging synchronously is quite slow, but maybe useful if you do not want
to spawn an extra thread.

By starting the logger, logging will be done asynchronously, messages
are added to a fifo, and a logger-thread will write the messages from the
fifo to the console and log files.

```cpp
// Start the logger-thread and change the log level.
logger_start(global_state_type::log_level_info);
```

### Wait-free logging
The `tt_log_*()` macros can log wait-free when all the following conditions
are met:

 - The logger-thread must be started to enable asynchronous logging.
 - The logger FIFO must not be full, 1024 messages fit in the FIFO.
 - The format-arguments must have a wait-free copy constructor.
 - The format-arguments are together not more than: 32 bytes.

### Accurate timestamps



Wait-free counting
------------------
Instead of logging, you may want to count how often a line of
code is executed. This is a pretty cheap and wait-free operation,
on x86-64 this is done with a single locked-increment instruction.

The following line of code increments a counter, the "my counter"
is the name you can give to a counter.

```cpp
++global_counter<"my counter">;
```

The `logger_start()` function should be called to display these counters
on a per minute interval.


