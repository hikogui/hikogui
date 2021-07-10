Change log
==========

0.4.0 Lovely Lizard
-------------------
This version is focused on making it practical for application developers
to start using the TTauri framework to create GUI application.

For this reason a lot of work has gone into improving the consistancy
between widgets and adding documentation and example code.

The GUI system itself and application instantiation has been simplified
to reduce the amount of unnessary preamble code.

Here are some of the important changes for this release:
 * The GUI system and widgets are easier to use and more consistant.
   - Simple unique pointer ownership of windows and widgets.
   - Widgets track delegates and callbacks using weak pointers.
   - Most widgets are non-template classes.
   - Simplified construction for widgets using the observer patern.
 * Added examples and documentation on how to use GUI widgets.
 * Easier to use optional subsystems initialization.
 * The event loop and rendering are now done on the same thread.
 * Minimum CPU requirements x86-64-v2: Sandy Bridge and Jaguar/Bulldozer
 * Reduced dependencies to only:
   - Vulkan SDK
   - CMake
   - C++20 compiler (MSVC)
   - Vulkan Memory Allocator (automatically installed by cmake or vcpkg)
   - Google Test (automatically installed by cmake)
 * And optional dependencies:
   - Doxygen
   - RenderDoc
   - vcpkg

0.3.0 Fancy Frog
----------------
In this version we concentrated on making it easy for developers
to install ttauri as a dependency for their own projects through vcpkg.

Here are some of the important changes for this release:
 * Removed the embedded dependencies and instead making them installable via vcpkg.
 * Made a vcpkg ttauri-port so that ttauri can be used as a dependency.
 * We've made several API usability improvements, such as:
   - making the widget's coordinate system relative to the widget;
   - now using high level geometric types such as vector, point and color in as many places as possible;
   - replacing the hidden state in the draw context to explicit arguments in draw calls; and
   - changing the cell-coordinate system when placing widget on a grid layout to Excel-like cell-coordinates.
 * Fixed several bugs.
 * Made several documentation improvements.
 * Improved the pull-request work flow with continuous integration requirements before merging.

