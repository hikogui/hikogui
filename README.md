TTauri GUI library [![Build on Windows](https://github.com/ttauri-project/ttauri/actions/workflows/build-on-windows.yml/badge.svg?branch=main)](https://github.com/ttauri-project/ttauri/actions/workflows/build-on-windows.yml) ![Version](https://img.shields.io/badge/dynamic/json?url=https://img.shields.io/badge/dynamic/json?url=https://raw.githubusercontent.com/ttauri-project/ttauri/master/vcpkg.json&label=version&query=$[%27version%27]&color=blue) [![License](https://img.shields.io/github/license/ttauri-project/ttauri.svg)](https://github.com/ttauri-project/ttauri/blob/master/LICENSE_1_0.txt)
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
 - Most or all drawing is GPU accelerated, using a Vulkan backend.
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

For hardware support see: [hardware\_support](docs/hardware_support.md)

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
