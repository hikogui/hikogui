Installing for ttauri development
=================================
This guide is how to install ttauri for developing ttauri itself.
For installing the ttauri library as part of an application see the
[ttauri_hello_world](https://github.com/ttauri-project/ttauri_hello_world)
example application.

Table of contents:
 - [Windows 10](#windows-10)

Windows 10
----------
### Install requirements:
 - The latest **Microsoft Visual Studio** or
   **Microsoft Visual Studio Preview** from <https://visualstudio.microsoft.com/>
   with the following options installed:
   - C++ core desktop features
   - C++ CMake tools for Windows
   - Test Adapter for Google Test
   - Windows 10 SDK
 - **git** from: <https://git-scm.com/>
 - The **Vulkan SDK** from: <https://www.lunarg.com/vulkan-sdk/>
 - optional: **vcpkg** from: <https://github.com/microsoft/vcpkg>
 - optional: **RenderDoc** (for Vulkan debugging) from: <https://renderdoc.org/>
 - optional: **Doxygen** (for documentation generation) from: <https://www.doxygen.nl/>

### Install vcpkg (optional)
TTauri will use the dependencies installed on the system, but it can use
vcpkg to install the dependencies if available.

From the "Developer Command Prompt for VS 2019":
```
c:\> cd c:\tools
c:\tools> git clone git@github.com:microsoft/vcpkg.git
Cloning into 'vcpkg'...
...

c:\tools\vcpkg> bootstrap-vcpkg.bat
Done.
...

c:\tools\vcpkg> vcpkg.exe integrate install --feature-flags=manifests
Applied user-wide integration for this vcpkg root.
CMake projects should use: "-DCMAKE_TOOLCHAIN_FILE=C:/tools/vcpkg/scripts/buildsystems/vcpkg.cmake"
...
```

### Clone this project
Clone the ttauri repository on your machine:

```
c:\Users\Tjienta\Projects> git clone git@github.com:ttauri-project/ttauri.git
Cloning into 'ttauri'...
...
```

### Building and running with "Microsoft Visual Studio"
You can then open the ttauri directory as a [directory-based project]
inside visual studio.

To build:
 1. Select `x64-MSVC-Debug` from the project `Configuration` pull down menu.
 2. `Project / Generate Cache` menu option
 3. `Build / Build All` menu option
 4. Select `ttauri_demo.exe` from the `Select Startup Item...` pull-down menu.
 5. `Debug / Start Debugging`

_Note: A "Window Security Alert" may show up, this is due to the RenderDoc API
creating a network server so that the RenderDoc application can remotely communicate
to the ttauri\_demo application._

[directory-based project]: https://docs.microsoft.com/en-us/visualstudio/ide/develop-code-in-visual-studio-without-projects-or-solutions?view=vs-2019

### Just-in-time debugging with Visual Studio
When a ttauri application hits a break-point while not running with a debugger the windows 10 CRT will
try to start the just-in-time debugger as a fallback.

See [Just in time debugging](https://docs.microsoft.com/en-us/visualstudio/debugger/debug-using-the-just-in-time-debugger?view=vs-2022).

A common problem in Windows 10 is missing registry entries for the jit debugger to work, set the following
two registry entries for 64 bit and 32 bit applications:

 * HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\AeDebug "Auto" DWORD (1)
 * HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\Windows NT\CurrentVersion\AeDebug "Auto" DWORD (1)

### Building and running using the "Developer Command Prompt for VS"

```
c:\Users\Tjienta\Projects> cd ttauri
c:\Users\Tjienta\Projects\ttauri> mkdir build
c:\Users\Tjienta\Projects\ttauri\build> cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON -DBUILD_SHARED_LIBS=OFF
c:\Users\Tjienta\Projects\ttauri\build> cmake --build . --config Debug
c:\Users\Tjienta\Projects\ttauri\build> cmake --install . --config Debug --prefix install
c:\Users\Tjienta\Projects\ttauri\build> install\tests\ttauri_tests.exe
c:\Users\Tjienta\Projects\ttauri\build> install\examples\ttauri_demo\ttauri_demo.exe
```

### Building and running using the "Developer Command Prompt for VS" with vcpkg

```
c:\Users\Tjienta\Projects> cd ttauri
c:\Users\Tjienta\Projects\ttauri> mkdir build
c:\Users\Tjienta\Projects\ttauri\build> cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON \
                                                 -DBUILD_SHARED_LIBS=OFF \
                                                 -DVCPKG_TARGET_TRIPLET=x64-windows-static \
                                                 -DCMAKE_TOOLCHAIN_FILE=C:/tools/vcpkg/scripts/buildsystems/vcpkg.cmake
c:\Users\Tjienta\Projects\ttauri\build> cmake --build . --config Debug
c:\Users\Tjienta\Projects\ttauri\build> cmake --install . --config Debug --prefix install
c:\Users\Tjienta\Projects\ttauri\build> install\tests\ttauri_tests.exe
c:\Users\Tjienta\Projects\ttauri\build> install\examples\ttauri_demo\ttauri_demo.exe
```
