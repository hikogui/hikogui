Contributing to the TTauri project
==================================

Issues
------
When you want to suggest a new feature, an improvement or file a bug
report you can do so through an [issue](https://github.com/ttauri-project/ttauri/issues).

The easiest way to contribute is by reporting issues with ttauri.
When reporting an issue with ttauri, make sure to clearly state:

 - The machine setup: "Windows 10 with RenderDoc installed."
 - The steps to reproduce: "I build ttauri in x64-MSVC-Debug, then run ttauri\_demo from Visual Studio"
 - The outcome you expected: "I expected to see log messages in the Output window"
 - The actual outcome: "I get no output at all" or "I get a exception at line 123 of logger.hpp"

Pull Requests
-------------
We are happy to accept pull requests for fixes, features and new widgets.
In order to avoid wasting your time, we highly encourage opening an issue to discuss
whether the PR you're thinking about making will be acceptable.

If you like to work on an already existing issue, you may want to assign
yourself to that issue before working on it, to reduce the chance of
two people working on the same pull request.

It could be helpful having a more real time discussion through discord at:
<https://discord.gg/7e8pFTsujw>

We have written down a [code style](code_style.md) which may help you
understand certain constructs in our code. Currently we are transitioning
to this new code style, so you may find some code that does not conform
to this, don't worry about this :smile:

Install
-------
Here is a description on how to install for the development of ttauri

### Windows 10

Install requirements:

 - The latest Microsoft Visual Studio Preview from <https://visualstudio.microsoft.com/vs/preview/>
   + C++ core desktop features
   + C++ CMake tools for Windows
   + Test Adapter for Google Test
   + Windows 10 SDK
 - git from <https://git-scm.com>
 - vcpkg from <https://github.com/microsoft/vcpkg> (see below for instructions)
 - Vulkan SDK from <https://www.lunarg.com/vulkan-sdk/>
 - optional: RenderDoc (for Vulkan debugging) from <https://renderdoc.org/>
 - optional: Doxygen (for documentation generation) from <https://www.doxygen.nl/>

The ttauri library builds the following of its dependencies using vcpkg:

 - date
 - fmt
 - Vulkan Memory Allocator

#### Developer Command Prompt for VS 2019

To install vcpkg, we will need to do the following
```

```

If you already have vcpkg installed you still will need to set VCPKG_ROOT and 'integrate'
each time you start a new shell.
```
c:\build>set VCPKG_ROOT=c:\tools\vcpkg
c:\build>call %VCPKG_ROOT%\vcpkg integrate install --feature-flags=manifests
```

```
c:\build>cmake -DVCPKG_TARGET_TRIPLET=x64-windows-static -DBUILD_SHARED_LIBS=OFF ..
```

```
c:\build>cmake --build .
```

Testing the demo application
----------------------------
There is a demo application included with the releases of ttauri.

It would be nice if people could test if this application will work on their computers.
And when there is a crash to create a mini-dump and send the mini-dump to the discord channel
"#demo-mini-dumps" <https://discord.gg/7e8pFTsujw> together with the version number
of the ttauri release.

To make crash mini-dump when the ttauri demo application crashes create the following registry key

`Computer\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps`

 Name         | Type                    | Value
 ------------ | ----------------------- | ------------
 DumpFolder   | Expandable String Value | "%LOCALAPPDATA%\CrashDumps"
 DumpCount    | DWORD (32-bit) Value    | 10
 DumpType     | DWORD (32-bit) Value    | 2


Code of Conduct
---------------
This project and everyone participating in it is governed by the
(TTauri Code of Conduct)[https://github.com/ttauri-project/ttauri/blob/main/docs/CODE_OF_CONDUCT.md]

