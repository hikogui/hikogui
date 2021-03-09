
[![Build status](https://ci.appveyor.com/api/projects/status/baqx98wu1ombp3d2/branch/main?svg=true)](https://ci.appveyor.com/project/takev/ttauri/branch/main)

TTauri GUI library
==================

A portable, low latency, retained-mode GUI library written in C++.

![Screenshot](docs/media/screenshots/demo_v0.2.0.gif)

Motivation
----------
I started this library to make a portable, low latency and modern looking
UI framework, which may also be used in proprietary (closed source) applications.

It is specifically designed to display information with low-latency,
and at the screen's refresh rate. Special care is taken for making
it easy for GUI element to observe and modify data external to the GUI.

Features
--------

 - Retained-mode GUI
 - Modern C++20 library.
 - Animation at the screen's refresh rate.
 - Most or all drawing is GPU accellerated, using a Vulkan backend.
 - Text is drawn using subpixel anti-aliasing and proper kerning.
 - High dynamic range and high gamut color handling.
 - High level API to make simple desktop applications.
 - Themes, including light/dark support.
 - Editable key-bindings.
 - Localization and translation.
 - Automatic application preferences storage.
 - Many support systems: logging, statistics, text-handling,
   text template language, expression language, dynamic type system.

Platform support
----------------
The following platforms are supported:
 - MSVC - Windows 10 - x64

Installation
------------
If you like to help with the development or want to modify ttauri you can
find instruction how to install the dependencies and how to build ttauri in the
[CONTRIBUTING](docs/CONTRIBUTING.md) document.

If you want to use ttauri as a linrary for your own application you can
find instructions in the [ttauri_hello_world](https://github.com/ttauri-project/ttauri_hello_world)
example application's [README](https://github.com/ttauri-project/ttauri_hello_world/blob/main/README.md).

Sponsors
--------
The following people and companies are platinum sponsors:

_There are currently no platinum sponsors._

for more sponsers please see [SPONSORS](SPONSORS.md).

