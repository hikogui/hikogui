Application lifetime
====================

This chapter describes how the lifetime of the application is handled.
From the moment that the user executes the application, until the application exits or aborts.

There are 5 main phases of the application lifetime:
 1. CRT Start
 2. Application setup
 3. Main Loop
 4. Exit
 5. CRT Finish

CRT Start
---------
There are two main tasks for the ttauri's CRT:

Configure the operating system's API for compatibility with the
ttauri library. In most cases this means turning off backward-compatibility, for
example:
 - On Windows 10 the CRT configures win32 to include leap-seconds in calls to
   the time related system calls.
 - On MacOS the CRT can tell the OS that the application can restore itself
   fully after being killed by the OS.

The second task is to call developer supplied `tt_main()` with cross platform
compatible parameters. On Windows 10 this means that the wide-string argument
list is unquoted, un-escaped and split into separate UTF-8 encoded arguments.

Application setup
-----------------
The application set begins when the developer defined `tt_main()`
is called by the CRT.

This is the part of the application where the developer can configure ttauri
and initialize subsystems before the main loop is entered.

### time\_stamp\_count (required)
The time\_stamp\_count subsystem calibrates the frequency of the TSC.
This calibration is needed to convert a TSC timestamp into a `hires_utc_clock::time_point`,
which in turn is used when logging messages to the console.

The CRT will start this subsystem and will take about 100 ms.

### hires\_utc\_clock (optional)
The hires\_utc\_clock subsystem increases the accuracy and performance
of converting a TSC to a `hires_utc_clock::time_point`.

Within a minute after starting this subsystem TSC to UTC conversion becomes
a much more accurate 1ppm, wait-free operation.

### Logger (optional)
The logger subsystem may be started by calling `tt::logger_start()`.

When the logger subsystem is running; logging becomes a wait-free operation.
On the current thread the data given with the log call is added to a queue,
while on the separate "logger" thread the message is formatted and written
to console and log file.

Logging is still possible without the logger subsystem,
but this is much slower; as formatting and writing to the console and log
file are done from the current thread.

