Contributing to the HikoGUI project
==================================

Issues
------

Bugs and other issues can be reported on the [Issue](https://github.com/hikogui/hikogui/issues)
page on github.

The easiest way to contribute is by reporting issues with HikoGUI.
When reporting an issue with HikoGUI, make sure to clearly state:

 - The machine setup: "Windows 10 with RenderDoc installed."
 - The steps to reproduce: "I build HikoGUI in x64-MSVC-Debug, then run hikogui\_demo from Visual Studio"
 - The outcome you expected: "I expected to see log messages in the Output window"
 - The actual outcome: "I get no output at all" or "I get a exception at line 123 of log.hpp"

Pull Requests
-------------

We are happy to accept pull requests for fixes, features and new widgets.
In order to avoid wasting your time, we highly encourage opening an
[Discussion](https://github.com/hikogui/hikogui/discussions) or an
[Issue](https://github.com/hikogui/hikogui/issues) to discuss
whether the PR you're thinking about making will be acceptable.

If you like to work on an already existing [Issue](https://github.com/hikogui/hikogui/issues),
you may want to assign yourself to that issue before working on it,
to reduce the chance of two people working on the same pull request.

It could be helpful having a more real time discussion through discord at:
[Society of TJ](https://discord.gg/CSddDuM) channel #HikoGUI/general.

We have written down a [code style](docs/code_style.md) which may help you
understand certain constructs in our code. Currently we are transitioning
to this new code style, so you may find some code that does not conform
to this, don't worry about this :smile:

Installation and Build Instructions
-----------------------------------
You can find the installation and build instructions for your favorite IDE
in [INSTALL.md](INSTALL.md).

Debugging with RenderDoc
------------------------

Debug builds of HikoGUI are linked against the RenderDoc API. Which means
that once an HikoGUI-application is started you can "Attach to running process"
and select the application there.

Since a HikoGUI-application tries to reduce the amount of window redraws; the
application may not show on this list, or you are unable to capture a frame
or the frame is not captured. You can force a redraw by selecting the
application window, or mouse-over the window.

Testing the demo application
----------------------------

There is a demo application included with the releases of HikoGUI.

It would be nice if people could test if this application will work on their computers.
And when there is a crash to create a mini-dump and send the mini-dump to the discord
[Society of TJ](https://discord.gg/CSddDuM) channel #HikoGUI/crash together with the version number
of the HikoGUI release.

To make crash mini-dump when the HikoGUI demo application crashes; create the following registry key:

`Computer\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps`

| Name       | Type                    | Value
|------------|-------------------------|-----------------------------
| DumpFolder | Expandable String Value | "%LOCALAPPDATA%\CrashDumps"
| DumpCount  | DWORD (32-bit) Value    | 10
| DumpType   | DWORD (32-bit) Value    | 2

External Bugs
-------------
There are a lot of bugs in IDEs and compilers with HikoGUI
[External Bugs](EXTERNAL_BUGS.md)

You may want to 'like' those bugs to hopefully get them resolved quicker.

Code of Conduct
---------------

This project and everyone participating in it is governed by the
[HikoGUI Code of Conduct](CODE_OF_CONDUCT.md)
