Dependencies
============

The current release dependencies are:

 - **Vulkan SDK** (required)
 - **Vulkan Memory Allocator**

Optional development dependencies are:

 - **Google Test** (testing)
 - **Doxygen** (documentation generator)
 - **RenderDoc** (interoperability)

Policy for adding dependencies
------------------------------

To make hikogui easy to integrate in your own application we try to limit
the number of dependencies to the absolute minimum.

 - We will allow libraries to be included which implement a preview of an
   accepted C++ standard feature.
 - Dependencies that are required to integrate with the operating system or
   integrate with another application are also allowed.
 - We will also allow dependencies that are required by the build environment.
 - Good quality support libraries.
