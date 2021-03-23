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

Subsystems to initialize:
 - **logger**: The logger subsystem may be started by calling `tt::logger_start()`.
   When the logger subsystem is running; a separate thread will log the messages
   to the console or log file. Logging is still possible without the logger subsystem,
   but this is much slower, as it is done from the current thread.
   

