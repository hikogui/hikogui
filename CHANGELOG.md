# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

 * Changed things.

## [0.8.1] - 2023-09-18 - Precious Polar Bear Deux

We've updated the CMake files quite a bit to improve installation with vcpkg:
 
 * Use RESOURCE property to install resource files.
 * Add functions to inherit RESOURCE files from a library, so that they will
   be installed along side an application.
 * Change HikoGUI's path module to use separate functions for getting locations
   to files and directories. `*_file()`, `*_dir()`, `*_dirs()`.
 * An application and the HikoGUI library can now detect if they are execution
   from the cmake-build directory instead of the install directory; and will
   automatically switch where the resource files are located. 

## [0.8.0] - 2023-09-06 - Precious Polar Bear

HikoGUI is now temporarily a header-only library. This is in preparation of
making it a module-only library. This required a lot of work to untangle
the many circular dependencies.

With modules it is now much easier to switch out different implementations
of classes and functions, so I've de-virtualized the GFX and GUI parts of
the library. This means that a win32-window and a X11-window are now both
called `hi::gui_window`. There are many benefits by fully replacing
implementations and having them have the same name, including allowing
implementation to have very different architectures and having them exposing
implementation specific members.

I've also been working on replacing the global classes with free-functions,
like the: font-book, theme-book, gui-system, audio-system, gfx-system, etc.
Right now some of them are still implemented as a class that is instantiated
as a global variable, but this is an implementation detail.

Another benefit of the header-only work is that the Unicode algorithms and
Unicode database are now constexpr. This includes improvement of the database
that now uses a double associative lookup O(1). The database is now stored
in columns, packed bit-wise, for improved space & cache efficiency.

The main text string type is now a `std::basic_string<hi::grapheme>`
also known as `hi::gstring`. The `hi::grapheme` contains one or more
Unicode code-points forming a Grapheme Cluster, a language-tag and a phrasing.
All of this information is needed to display text properly, and for
possible features like spell checking and text-to-speech.

I discovered and fixed a security vulnerability with mapping files and parsing,
in particular with my implementation of true-type files. True-type files are
designed to be loaded in memory and be directly used as a data structure.
HikoGUI uses `mmap()` to map a true-type file in memory, check if the data
is valid, and then use the data directly. However when you memory map a file the
file can be changed during the execution of the application, bypassing the
validation check that is only done when the file was first mapped. The fix
is to simply validate all data read during the execution of the application.
There is a performance impact, but it is a small price to pay compared to
loading the true-type file in memory, especially on low memory devices.

Of course I also decided to rewrite the true-type font parser and allow it to
render runs of text. A run is a piece of text that is part of a single line, and
uses the same font, style and language. In the future we can use the GPOS
and GSUB tables to properly handle ligatures and cursive fonts. The text-shaper
will need to relegate the responsibilities of rendering text to different
sub-systems.

Added a Vulkan triangle-example, which shows how to use HikoGUI together with
an application that wants to render graphics. Basically the application
can directly render into the swap-chain images through a call-back mechanism.
Then HikoGUI draws the GUI on-top of this drawing using alpha compositing, where
the widget that shows the drawing punches a hole through the GUI so that the
drawing stays visible.

## [0.7.0] - 2022-10-07 - Strange Squirrel

To make it possible to select and configure audio devices this release
is a start to make composable shared-state, composable preferences and
composable widgets.

There are also a few other systems that have been updated:

 * Co-routine; awaiting on a timeout.
 * Simplified widget event handling.
 * The `color` type can hold a RGBA color value or be a semantic enum value.
 * Handle HDR on platforms with uniform HDR/SDR (windows 10 doesn't).
 * Very fast 2 step index-array unicode character lookup.
 * New grapheme type uses 21-bits and can be used as char of std::basic\_string.
 * New user-extensible char-encoding conversion system.
 * std::format early static-type-check on `hi_log*()` macros.
 * Add `mode` enumeration to widgets to handle visibility, disabled, enabled.
 * Add a baseline system to align text between widgets on the same row.
 * Improve `observer<>` to make sub-observers from member variables of the
   observed type.
 * Audio device capability interrogation system.
 * Reduce use of `URL` and use `std::filesystem::path` where appropriate.
 * Rewritten `file` and `file_view` to function as copyable and movable value types.
 * Split INSTALL documentation between different IDEs.

## [0.6.0] - 2022-04-12 - Dizzy Donkey

The changes in this version are pretty random, but there is a theme
of improving how to create custom widgets.

 * The ttauri project is renamed to HikoGUI.
 * The drawing API is more consistent and capable; for example adding
   color gradients, convex quads and allowing glyphs to be overlapped.
 * New text-shaper and text widget which handles bidirectional text,
   with bidirectional-double cursors, multiple paragraphs, and sharper
   rendering by scaling and positioning to sub-pixel boundaries.
 * Improved grid widget which now supports row- and column-spans.
 * New grapheme-cluster type that can be used inside std::basic\_string.
 * An central API to retrieve user-settings from the operating system,
   including a notifier for the application to directly react on changes.
 * DPI scaling is implemented by scaling the measurements of a theme.
 * Multi-monitor support for positioning and maximizing.
 * co-routine `task` and `scoped_task` which can await on notifiers and
   observables to handle complex user interface interactions and window
   ownership.
 * New blocking main event-loop with more accurate vertical-sync support;
   improving animations even in debug builds.
 * Improved localization support with language\_tags that have access to
   ISO-codes for language, script and region.


## [0.5.1] - 2021-10-11 - Bad Butterfly

 * Fix bug; pre-main initialization order of global\_counter.
 * Make it easier to build out-of-tree application without vcpkg.
 * vcpkg builds are not recommended at the moment due to future
   changes of std::format and std::ranges to the c++20 standard.
   Please build using non-vcpkg to ensure equal versions of CMake
   and the compiler.

## [0.5.0] - 2021-09-29 - Eager Elephant

The changes in this version are pretty random, but there is a theme
of improving the application developer's view of the API.

 * Removing the singletons of gui\_system, gfx\_system, audio\_system,
   vertical\_sync, theme\_book, font\_book, keyboard\_bindings and
   preferences. These are no longer owned by the ttauri library, instead
   they may be owned by local variables in the application.
 * Improve performance of the font-book by letting the application hold a
   reference to individual fonts directly.
 * Wrote a how-to for logging, counting and tracing:
   - Improved performance of counters.
   - Improved performance of tracing.
   - Logging is now done in local time.
   - Replaced `hi::hires_utc_clock` with
     `std::chrono::utc_time<std::chrono::nanoseconds>`
   - Reimplemented CPU-timestamp-counter conversion using
     `std::chrono::utc_clock`.
 * Wrote a how-to for application preference:
   - Implemented JSON-path for selecting values in a JSON file.
   - Reimplemented a robust UTF-8, UTF-16, UTF-32 codec for properly
     handling invalid encoded text strings.
   - Reimplemented dynamic data type `hi::datum`.
   - Implement `hi::pickle` system to convert between custom types and
     `hi::datum`.
   - Reimplemented observables with better automatic ownership model and
     better callback handling.
   - Multiple preference-files may now be opened at the same time.
 * Preparing for a how-to for writing custom widgets:
   - Replaced flat shader with a better optimized rounded box shader.
 * Add address-sanitizer builds.
 * Finalize BON8 (Binary Object Notation 8) specification.

## [0.4.0] - 2021-07-10 - Lovely Lizard

This version is focused on making it practical for application developers to
start using the ttauri framework to create GUI application.

For this reason a lot of work has gone into improving the consistency between
widgets and adding documentation and example code.

The GUI system itself and application instantiation has been simplified to
reduce the amount of unnecessary preamble code.

Here are some of the important changes for this release:
* The GUI system and widgets are easier to use and more consistent.
  - Simple unique pointer ownership of windows and widgets.
  - Widgets track delegates and callbacks using weak pointers.
  - Most widgets are non-template classes.
  - Simplified construction for widgets using the observer pattern.
* Added examples and documentation on how to use GUI widgets.
* Easier to use optional subsystems initialization.
* The event loop and rendering are now done on the same thread.
* Minimum CPU requirements x86-64-v2: Sandy Bridge and Jaguar/Bulldozer
* Reduced dependencies to only:
  - Vulkan SDK
  - CMake
  - C++20 compiler (MSVC)
  - Vulkan Memory Allocator (automatically installed by CMake or vcpkg)
  - Google Test (automatically installed by CMake)
* And optional dependencies:
  - Doxygen
  - RenderDoc
  - vcpkg

## [0.3.0] - 2021-03-17 - Fancy Frog

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

## [0.2.0] - 2021-02-17 - First Public Release

The first public release.

<!-- Section for Reference Links -->

[Unreleased]: https://github.com/hikogui/hikogui/compare/v0.8.1...HEAD
[0.8.1]: https://github.com/hikogui/hikogui/compare/v0.8.0...v0.8.1
[0.8.0]: https://github.com/hikogui/hikogui/compare/v0.7.0...v0.8.0
[0.7.0]: https://github.com/hikogui/hikogui/compare/v0.6.0...v0.7.0
[0.6.0]: https://github.com/hikogui/hikogui/compare/v0.5.1...v0.6.0
[0.5.1]: https://github.com/hikogui/hikogui/compare/v0.5.0...v0.5.1
[0.5.0]: https://github.com/hikogui/hikogui/compare/v0.4.0...v0.5.0
[0.4.0]: https://github.com/hikogui/hikogui/compare/v0.3.0...v0.4.0
[0.3.0]: https://github.com/hikogui/hikogui/compare/v0.2.0...v0.3.0
[0.2.0]: https://github.com/hikogui/hikogui/releases/tag/v0.2.0
