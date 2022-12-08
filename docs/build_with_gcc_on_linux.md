Building with gcc on Linux
==========================

Install requirements
--------------------
 - Latest dev-tool of the distribution
   - gcc > 13
   - CMake
   - git
 - The **Vulkan SDK** from: <https://www.lunarg.com/vulkan-sdk/>
 - optional: **RenderDoc** (for Vulkan debugging) from: <https://renderdoc.org/>
 - optional: **Doxygen** (for documentation generation) from: <https://www.doxygen.nl/>

Clone the HikoGUI project from github
-------------------------------------
Clone the HikoGUI repository on your machine:

```bash
git clone git@github.com:hikogui/hikogui.git
```

Install Vulkan SDK
------------------

```bash
cd /opt
mkdir vulkan
cd vulkan
get https://sdk.lunarg.com/sdk/download/1.3.231.2/linux/vulkansdk-linux-x86_64-1.3.231.2.tar.gz
tar -xzvf vulkansdk-linux-x86_64-1.3.231.2.tar.gz
export VULKAN_SDK=/opt/vulkan/1.3.231.2/x86_64
```

Building HikoGUI
----------------

```bash
export VULKAN_SDK=/opt/vulkan/1.3.231.2/x86_64
cd hikogui
mkdir build
cd build

cmake .. -DCMAKE_VERBOSE_MAKEFILE=ON

cmake --build . --config Debug

cmake --install . --config Debug --prefix install
```

```bash
install\tests\hikogui_tests

install\examples\hikogui_demo\hikogui_demo
```

