Installing for hikogui development
=================================

This guide explains how to install hikogui for developing hikogui itself.
For installing the hikogui library as part of an application see the
[hikogui_hello_world](https://github.com/hikogui/hikogui_hello_world)
example application.

Table of contents:
 - [Windows 10](\#windows-10)

Windows 10
----------

### Install requirements:

 - The latest **Microsoft Visual Studio** or
   **Microsoft Visual Studio Preview** from <https://visualstudio.microsoft.com/>
   with the following options installed:
   - C++ core desktop features
   - C++ CMake tools
   - Test Adapter for Google Test
   - Windows 10 SDK
 - **git** from: <https://git-scm.com/>
 - The **Vulkan SDK** from: <https://www.lunarg.com/vulkan-sdk/>
   with the following options installed:
   - The Vulkan SDK Core (Always Installed)
   - Vulkan Memory Allocator header.
 - optional: **vcpkg** from: <https://github.com/microsoft/vcpkg>
 - optional: **RenderDoc** (for Vulkan debugging) from: <https://renderdoc.org/>
 - optional: **Doxygen** (for documentation generation) from: <https://www.doxygen.nl/>

### Install vcpkg (optional)

HikoGUI will use the dependencies installed on the system, but it can use
vcpkg to install the dependencies if available.

Run the following commands from the "Developer Command Prompt" to install vcpkg:

```bash
git clone git@github.com:microsoft/vcpkg.git
```

```bash
cd vcpkg
bootstrap-vcpkg.bat
```

```bash
vcpkg.exe integrate install --feature-flags=manifests
```

You should get the following message:

```text
Applied user-wide integration for this vcpkg root.
CMake projects should use: "-DCMAKE_TOOLCHAIN_FILE=C:/<path>/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

### Clone this project

Clone the hikogui repository on your machine:

```bash
git clone git@github.com:hikogui/hikogui.git
```

### Building and running with "Microsoft Visual Studio"

You can then open the hikogui directory as a [directory-based project]
inside visual studio.

To build:
 1. Select `x64-MSVC-Debug` from the project `Configuration` pull down menu.
 2. `Project / Generate Cache` menu option
 3. `Build / Build All` menu option
 4. Select `hikogui_demo.exe` from the `Select Startup Item...` pull-down menu.
 5. `Debug / Start Debugging`

_Note: A "Window Security Alert" may show up, this is due to the RenderDoc API
creating a network server so that the RenderDoc application can remotely communicate
to the hikogui_demo application._

[directory-based project]: https://docs.microsoft.com/en-us/visualstudio/ide/develop-code-in-visual-studio-without-projects-or-solutions?view=vs-2019

### Building and running using the "Developer Command Prompt"

```bash
cd hikogui
mkdir build
cd build

cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON -DBUILD_SHARED_LIBS=OFF

cmake --build . --config Debug

cmake --install . --config Debug --prefix install
```

```bash
install\tests\hikogui_tests.exe

install\examples\hikogui_demo\hikogui_demo.exe
```

### Building and running using the "Developer Command Prompt" with vcpkg

```
cd hikogui
mkdir build
cd build

cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON \
         -DBUILD_SHARED_LIBS=OFF \
         -DVCPKG_TARGET_TRIPLET=x64-windows-static \
         -DCMAKE_TOOLCHAIN_FILE=C:/tools/vcpkg/scripts/buildsystems/vcpkg.cmake

cmake --build . --config Debug

cmake --install . --config Debug --prefix install
```

```bash
install\tests\hikogui_tests.exe

install\examples\hikogui_demo\hikogui_demo.exe
```

### Just-in-time debugging with Visual Studio

When a hikogui application hits a break-point while not running with a debugger the Windows 10 CRT will
try to start the just-in-time debugger as a fallback.

A common problem in Windows 10 are the missing registry entries for the jit debugger to work.

The fix is to add a `DWORD Value` of `Auto`, with `Value data` of `1`, to the following registry keys:

 * `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AeDebug` - DWORD "Auto" : Value (1)
 * `HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows NT\CurrentVersion\AeDebug` - DWORD "Auto" : Value (1)

See [Just in time debugging](https://docs.microsoft.com/en-us/visualstudio/debugger/debug-using-the-just-in-time-debugger?view=vs-2022).
