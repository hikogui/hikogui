Building with Visual Studio and vcpkg 
=====================================

Install requirements
--------------------
 - The latest **Microsoft Visual Studio** or
   **Microsoft Visual Studio Preview** from <https://visualstudio.microsoft.com/>
   with the following options installed:
   - C++ core desktop features
   - C++ CMake tools
   - Test Adapter for Google Test
   - Windows 10 (or newer) SDK
 - **git** from: <https://git-scm.com/>
 - The **Vulkan SDK** from: <https://www.lunarg.com/vulkan-sdk/>
   with the following options installed:
   - The Vulkan SDK Core (Always Installed)
   - Vulkan Memory Allocator header.
 - **vcpkg** from: <https://github.com/microsoft/vcpkg>
 - optional: **RenderDoc** (for Vulkan debugging) from: <https://renderdoc.org/>
 - optional: **Doxygen** (for documentation generation) from: <https://www.doxygen.nl/>

Install vcpkg
-------------
HikoGUI will use the dependencies installed on the system, but it can use
vcpkg to install the dependencies if available.

Run the following commands from the "Developer Command Prompt" to install vcpkg:

```bash
git clone git@github.com:microsoft/vcpkg.git C:\tools\vcpkg
cd C:\tools\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg.exe integrate install --feature-flags=manifests
```

You should get the following message:

```text
Applied user-wide integration for this vcpkg root.
CMake projects should use: "-DCMAKE_TOOLCHAIN_FILE=C:/<path>/vcpkg/scripts/buildsystems/vcpkg.cmake"
```

If you do not install vcpkg in `C:\tools\vcpkg` then you will need to edit `CMakePresets.json`.

Clone the HikoGUI project from github
-------------------------------------
Clone the HikoGUI repository on your machine:

```bash
git clone git@github.com:hikogui/hikogui.git
```

Building and running
--------------------
You can then open the `hikogui` directory as a [directory-based project]
inside visual studio.

To build:
 1. Select `x64-vcpkg-MSVC-Debug` from the project `Configuration` pull down menu.
 2. `Project / Generate Cache` menu option
 3. `Build / Build All` menu option
 4. Select `hikogui_demo.exe` from the `Select Startup Item...` pull-down menu.
 5. Select `Build Only` in the `Error List` window to ignore IntelliSense false positives.
 6. `Debug / Start Debugging`

_Note: A "Window Security Alert" may show up, this is due to the RenderDoc API
creating a network server so that the RenderDoc application can remotely communicate
to the hikogui_demo application._

Other configurations that are possible to build:
 - `x64-vcpkg-MSVC-Debug` Make a debug build
 - `x64-vcpkg-MSVC-ReleaseWithDebugInfo` Make an fully optimized build
 - `x64-vcpkg-MSVC-Release` Make a fully optimized build and build all examples and demos.
 - `x64-vcpkg-MSVC-Analysis` Build with static-analysis.

Common build problems:
 - When Visual Studio gets confused try deleting the CMake cache using the
  `Project / Delete Cache and Reconfigure` menu option.
 - If you get "permission denied", this may be due to a still running
   process like a hung hikogui_test.exe. Terminate that process using the
   Task Manager.
 - You may also get "permission denied" when during building a compiler-crash
   causes a file to be written without any permissions associated at all.
   This can be solved by rebooting the computer and followed by deleting the CMake cache.
 - The Test Explorer may get confused as well, try deleting the cache in:
   `hikogui\.vs\v17\TestStore\0\*.testlog`.

[directory-based project]: https://docs.microsoft.com/en-us/visualstudio/ide/develop-code-in-visual-studio-without-projects-or-solutions?view=vs-2019

Building and running using the "Developer Command Prompt"
---------------------------------------------------------
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

Just-in-time debugging with Visual Studio
-----------------------------------------
When a HikoGUI application hits a break-point while not running with a debugger the Windows 10 CRT will
try to start the just-in-time debugger as a fallback.

A common problem in Windows 10 are the missing registry entries for the JIT-debugger to work.

The fix is to add a `DWORD Value` of `Auto`, with `Value data` of `1`, to the following registry keys:

 * `HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AeDebug` - DWORD "Auto" : Value (1)
 * `HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows NT\CurrentVersion\AeDebug` - DWORD "Auto" : Value (1)

See [Just in time debugging](https://docs.microsoft.com/en-us/visualstudio/debugger/debug-using-the-just-in-time-debugger?view=vs-2022).

_Note due to bugs in IntelliSense you will get JIT-debugger prompts for Visual Studio itself. This does
allow you to create crash-dumps for Visual Studio and its components to report bugs with Microsoft._
