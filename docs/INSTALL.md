Installing for ttauri development
=================================
This guide is how to install ttauri for developing ttauri itself.
For installing the ttauri library as part of an application see the
[ttauri_hello_world](https://github.com/ttauri-project/ttauri_hello_world)
example application.

Table of contents:
 - [MS Visual Studio with vcpkg](#ms-visual-studio-with-vcpkg)
 - [MS Visual Studio without vcpkg](#ms-visual-studio-without-vcpkg)

MS Visual Studio with vcpkg
---------------------------
### Install requirements:
 - The latest **Microsoft Visual Studio** or
   **Microsoft Visual Studio Preview** from <https://visualstudio.microsoft.com/>
   with the following options installed:
   - C++ core desktop features
   - C++ CMake tools for Windows
   - Test Adapter for Google Test
   - Windows 10 SDK
 - git from: <https://git-scm.com/>
 - The Vulkan SDK from: <https://www.lunarg.com/vulkan-sdk/>
 - vcpkg from: <https://github.com/microsoft/vcpkg>
 - optional: RenderDoc (for Vulkan debugging) from: <https://renderdoc.org/>
 - optional: Doxygen (for documentation generation) from: <https://www.doxygen.nl/>

### Install vcpkg

From the "Developer Command Prompt for VS 2019":
```
c:\> cd c:\tools
c:\tools>git clone git@github.com:microsoft/vcpkg.git
Cloning into 'vcpkg'...
...

c:\tools\vcpkg>bootstrap-vcpkg.bat
Downloading https://github.com/microsoft/vcpkg-tool/releases/download/2021-02-24-d67989bce1043b98092ac45996a8230a059a2d7e/vcpkg.exe -> C:\tools\vcpkg/vcpkg.exe
Done.
...

c:\tools\vcpkg>vcpkg.exe integrate install --feature-flags=manifests
Applied user-wide integration for this vcpkg root.
...
```

### Clone this project

Clone the ttauri repository on your machine:

```
c:\Users\Tjienta\Projects>git clone git@github.com:ttauri-project/ttauri.git
Cloning into 'ttauri'...
...
```

### Building and running with Visual Studio

You can then open the ttauri directory as a [directory-based project]
inside visual studio.

To build:
 1. Select `x64-MSVC-Debug` from the project `Configuration` pull down menu.
 2. `Project / Generate Cache` menu option
 3. `Build / Build All` menu option
 4. Select `ttauri\_demo.exe` from the `Select Startup Item...` pull-down menu.
 5. `Debug / Start Debugging`

_Note: A "Window Security Alert" may show up, this is due to the RenderDoc API
creating a network server so that the RenderDoc application can remotely communicate
to the ttauri\_demo application._

MS Visual Studio without vcpkg
------------------------------

### Install requirements:
 - The latest **Microsoft Visual Studio** or
   **Microsoft Visual Studio Preview** from <https://visualstudio.microsoft.com/>
   with the following options installed:
   - C++ core desktop features
   - C++ CMake tools for Windows
   - Test Adapter for Google Test
   - Windows 10 SDK
 - git from: <https://git-scm.com/>
 - The Vulkan SDK from: <https://www.lunarg.com/vulkan-sdk/>
 - optional: RenderDoc (for Vulkan debugging) from: <https://renderdoc.org/>
 - optional: Doxygen (for documentation generation) from: <https://www.doxygen.nl/>

### Clone this project

Clone the ttauri repository on your machine:

```
c:\Users\Tjienta\Projects>git clone git@github.com:ttauri-project/ttauri.git
Cloning into 'ttauri'...
...
```

### Building and running with Visual Studio

You can then open the ttauri directory as a [directory-based project]
inside visual studio.

To build:
 1. Select `x64-MSVC-Debug` from the project `Configuration` pull down menu.
 2. `Project / Generate Cache` menu option
 3. `Build / Build All` menu option
 4. Select `ttauri\_demo.exe` from the `Select Startup Item...` pull-down menu.
 5. `Debug / Start Debugging`

_Note: A "Window Security Alert" may show up, this is due to the RenderDoc API
creating a network server so that the RenderDoc application can remotely communicate
to the ttauri\_demo application._
