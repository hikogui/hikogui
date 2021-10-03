TTauri GUI library [![Build on Windows](https://github.com/ttauri-project/ttauri/actions/workflows/build-on-windows.yml/badge.svg?branch=main)](https://github.com/ttauri-project/ttauri/actions/workflows/build-on-windows.yml) [![Version](https://img.shields.io/badge/dynamic/json?url=https://raw.githubusercontent.com/ttauri-project/ttauri/main/vcpkg.json&label=Latest%20Version&query=$[%27version%27]&color=blue)](https://github.com/ttauri-project/ttauri/releases/latest) [![License](https://img.shields.io/github/license/ttauri-project/ttauri.svg)](https://github.com/ttauri-project/ttauri/blob/main/LICENSE_1_0.txt)
==================

Current releases are broken
---------------------------
Due to Visual Studio removing std::format from C++20, and cmake changing the compiler flag for C++20
ttauri releases will be broken until Visual Studio 2022 is officially released. After
Visual Studio 2022 we should be able to make c++23 builds which should work for some time.
However in the future it will break again due to ABI changes of std::format.


A portable, low latency, retained-mode GUI framework written in C++
-------------------------------------------------------------------

I started this library to make a portable, low latency and modern looking
UI framework, which may also be used in proprietary (closed source) applications.

It is specifically designed to display information with low-latency,
and at the screen's refresh rate. Special care is taken for making
it easy for GUI element to observe and modify data external to the GUI.

You can find a lot more information,
[documentation](https://www.ttauri-project.org/docs/ttauri/main/index.html),
[example code](https://github.com/ttauri-project/ttauri_hello_world/blob/main/src/main.cpp),
news and blog posts on the main web site: <https://www.ttauri-project.org/>

Features
--------

 - High level API to make simple desktop applications.
 - Modern C++20 library.
 - Retained-mode GUI.
 - GUI will dynamically track the state of the application.
 - Localization and translation.
 - Animation at the screen's refresh rate.
 - Themes; including light/dark support.
 - Editable key-bindings.

![Themes with dark and light mode](docs/media/screenshots/demo_dark_and_light.png)

 - Most or all drawing is GPU accelerated with Vulkan.
 - Text is drawn using kerning, perceptional correct blending and subpixel anti-aliasing.
 - High dynamic range and high gamut color handling.

![Subpixel anti-aliasing](docs/media/screenshots/subpixel_glyphs.png)

 - Automatic application preferences storage.
 - Many support systems:
   + logging,
   + statistics,
   + text handling,
   + text template language,
   + expression language,
   + dynamic type system.

Example
-------
Here is some example code for an application with three radio buttons,
who form a set by sharing a single `value` observable.

```
int tt_main(int argc, char *argv[])
{
    observable<int> value = 0;

    auto &window = gui_system::global().make_window(l10n("Radio button example"));
    window.content().make_widget<label_widget>("A1", l10n("radio buttons:"));
    window.content().make_widget<radio_button_widget>("B1", l10n("one"), value, 1);
    window.content().make_widget<radio_button_widget>("B2", l10n("two"), value, 2);
    window.content().make_widget<radio_button_widget>("B3", l10n("three"), value, 3);

    return gui_system::global().loop();
}
```

Platform support
----------------

The following platforms are supported:

 - MSVC - Windows 10 - x64

For hardware support see: [hardware\_support](docs/hardware_support.md)

Installation
------------

If you like to help with the development or want to modify ttauri you can
find instruction how to install the dependencies and how to build ttauri in the
[CONTRIBUTING](docs/CONTRIBUTING.md) document.

If you want to use ttauri as a library for your own application you can
find instructions in the [ttauri_hello_world](https://github.com/ttauri-project/ttauri_hello_world)
example application's [README](https://github.com/ttauri-project/ttauri_hello_world/blob/main/README.md).

Sponsors
--------

The following people and companies are platinum sponsors:

_There are currently no platinum sponsors._

for more sponsors please see [SPONSORS](SPONSORS.md).

