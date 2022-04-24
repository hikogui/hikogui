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
There are two main tasks for the hikogui's CRT:

Configure the operating system's API for compatibility with the
hikogui library. In most cases this means turning off backward-compatibility, for
example:

- On MacOS the CRT can tell the OS that the application can restore itself
  fully after being killed by the OS.

The second task is to call developer supplied `hi_main()` with cross platform
compatible parameters. On Windows 10 this means that the wide-string argument
list is unquoted, un-escaped and split into separate UTF-8 encoded arguments.

Application setup
-----------------
The application set begins when the developer defined `hi_main()`
is called by the CRT.

This is the part of the application where the developer can configure hikogui
and initialize subsystems before the main loop is entered.
