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

To install vcpkg, we will need to do the following:
```
c:\tools>git clone https://github.com/microsoft/vcpkg
c:\tools>cd vcpkg
c:\tools\vcpkg>bootstrap-vcpkg.bat
c:\tools\vcpkg>vcpkg integrate install --feature-flags=manifests
```

To clone ttauri:
```
c:\Users\Tjienta\Projects>git clone https://github.com/ttauri-project/ttauri
```

#### Visual Studio 2019
You can open ttauri directly in Visual Studio using the
"[open folder](https://docs.microsoft.com/en-us/cpp/build/open-folder-projects-cpp?view=msvc-160)" method.

Since it is a large project you may have to wait a minute before Visual Studio is finished with fully loading
the project. Once it is loaded you will see a selection box with a set of build configurations:
 - x64-MSVC-Debug
 - x64-MSVC-Release
 - x64-MSVC-ReleaseWithDebugInfo
 - x64-Clang-Debug (currently not supported)
 - x64-Clang-Release (currently not supported)
 - x64-Clang-ReleaseWithDebugInfo (currently not supported)

Select the x64-MSVC-Debug and use the following menu items to build the project:
 - Project / Generate Cache for ttauri
 - Build / Build All

After building you can select "ttauri_demo.exe" from "Select Startup Item..." next to the run-button. Then
press that button to run the debug build (with the debugger attached).

You may also want to read the following about how to use CMake projects with visual studio:
<https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-160>

#### Developer Command Prompt for VS 2019
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

### Gentoo Linux
**Note: TTauri does not currently build fully on Linux.**

The following packages need to be installed:

 - app-arch/zip (needed for vcpkg)
 - dev-util/vulkan-tools (the vulkan SDK)
 - dev-util/glslang
 - media-libs/shaderc (for building the shaders)

Since the vulkan sdk is part of gentoo it will be installed in `/usr`.

```
ttauri> mkdir build
ttauri> cd build
build> export VCPKG_ROOT=~/src/vcpkg
build> export VULKAN_SDK=/usr
build> $VCPKG_ROOT/vcpkg integrate install --feature-flags=manifests
build> cmake -DVCPKG_TARGET_TRIPLET=x64-linux -DBUILD_SHARED_LIBS=ON ..
build> cmake --build .
```

Debugging with RenderDoc
------------------------
Debug builds of ttauri are linked against the RenderDoc API. Which means
that once an ttauri-application is started you can "Attach to running process"
and select the application there.

Since a ttauri-application tries to reduce the amount of window redraws; the
application may not show on this list, or you are unable to capture a frame
or the frame is not captured. You can force a redraw by selecting the
application window, or mouse-over the window.

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

