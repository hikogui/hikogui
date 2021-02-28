Dependencies
============

The current dependencies are:

 - **Vulkan SDK** 
 - **Vulkan Memory Allocator**
 - **date**
 - **fmt**
 - **tzdata**
 - **Google Test**
 - **Doxygen**
 - **RenderDoc**

Policy for adding dependencies
------------------------------
To make ttauri easy to integrate in your own application we try to limit
the number of dependencies to the absolute minimum.

We will allow libraries to be included which implement a preview of an
accepted c++ standard feature. Current that includes: fmt, date and tzdata.

Dependencies that are required to integrate with the operating system or
integrate with another application are also allowes: such as Vulkan and RenderDoc
 
We will also allow dependencies that are required by the build environment,
For example visual studio in cmake-mode will only work correctly with
Google Test or Boost Test.
<https://developercommunity.visualstudio.com/t/ctest-test-adapter-for-test-explorer-add-custom-te-1/640938>

The odd one out in the list above is the Vulkan Memory Allocator, which
technically could have been written from scratch in ttauri; maybe we will in time.
However maybe one day it will become part of the Vulkan SDK.


