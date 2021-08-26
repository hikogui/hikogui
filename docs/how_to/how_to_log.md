How To Log
==========

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

For performance reasons the actual formatting of the text is delayed and
done on the logger thread. Logging is wait-free as long as the log queue
is not full and all the data being logged fits within a log queue slot of
about 40 bytes.

### Available log macros:

 - `tt_log_fatal()` - _also terminates the application._
 - `tt_log_error()`
 - `tt_log_warning()`
 - `tt_log_info()`
 - `tt_log_debug()`
 - `tt_log_audit()`

### Asynchronous or synchronous logging


### Changing the log level

Counting
--------


