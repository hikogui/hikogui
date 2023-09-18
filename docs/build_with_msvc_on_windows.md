Building with MSVC on Windows
=============================

Install requirements
--------------------
For the best experience install Visual Studio before Visual Studio Code.

 - The latest **Microsoft Visual Studio (Preview)** from <https://visualstudio.microsoft.com/>
   with the following options installed:
   - C++ core desktop features
   - C++ CMake tools
   - Windows 10 (or newer) SDK
 - **git** from: <https://git-scm.com/>
 - The **Vulkan SDK** from: <https://www.lunarg.com/vulkan-sdk/>
   with the following options installed:
   - The Vulkan SDK Core (Always Installed)
   - Vulkan Memory Allocator header.
 - optional: **RenderDoc** (for Vulkan debugging) from: <https://renderdoc.org/>
 - optional: **Doxygen** (for documentation generation) from: <https://www.doxygen.nl/>

Setup Visual Studio
-------------------
We offer a curated list of required Visual Studio components in `.github\.vsconfig`.
You can import this file using the Visual Studio Installer.

Clone the HikoGUI project from github
-------------------------------------
Clone the HikoGUI repository on your machine:

```sh
git clone git@github.com:hikogui/hikogui.git
```

Building HikoGUI
----------------
Use the *x64 Native Tools Command Prompt for VS 2022 Preview* as the command
prompt as it will configure the environment variables which are needed for
CMake to build using MSVC.

```sh
cd hikogui
mkdir build
cd build

cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=../install -DBUILD_TESTING=OFF -DBUILD_EXAMPLES=OFF ..

cmake --build .

cmake --install .
```


