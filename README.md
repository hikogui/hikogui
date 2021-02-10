TTauri GUI library
==================

A modern looking, portable, high performance, retained-mode GUI library
written in C++.


Motivation
----------
I started this library to make a portable, high performance and modern looking
UI framework, which can be used in proprietary (closed source) applications.

It is specifically designed to display information with low-latency,
and at the screen's refresh rate. Special care is taken for making
it easy for GUI element to observe and modify data external to the
GUI.

Features
--------

 - Retained-mode GUI, implemented as a game setup-update-draw loop.
 - Modern C++20 library.
 - Animation at the screen's refresh rate.
 - Information is displayed at low latency.
 - Most or all drawing is GPU accellerated, using a Vulkan backend.
 - Text is drawn using subpixel anti-aliasing and proper kerning.
 - High dynamic range and high gamut color handling.
 - High level API to make simple desktop or mobile applications.
 - Themes and light/dark support.
 - Key-bindings.
 - Localization and translation.
 - Automatic application preferences storage.

Platform support
----------------
The following platforms are supported:
 - MSVC - Windows 10 - x64

Code Example
------------

Installation
------------

### MSVC - Windows 10

Install requirements:
 - The latest Microsoft Visual Studio Preview
   - The google-test framwork and Google Test Adapter
   - CMake
 - The Vulkan SDK from: https://www.lunarg.com/vulkan-sdk/
 - the latest Python 3.x.x from: https://www.python.org/downloads/windows/
 - optional: RenderDoc (for Vulkan debugging) from: https://renderdoc.org/
 - optional: Doxygen (for documentation generation)

Your CMake project should include ttauri as a subdirectory in your project
and add it to your CMake project like this example:

```CMakeLists.txt
subdirectory(ttauri)

add_executable(my_program my_main.cpp)
target_link_libraries(my_program PRIVATE ttauri)
```

You can then open your project as a "directory-based project" inside visual
studio. See: https://docs.microsoft.com/en-us/visualstudio/ide/develop-code-in-visual-studio-without-projects-or-solutions?view=vs-2019


