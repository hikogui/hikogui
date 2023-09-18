Install the HikoGUI development environment
===========================================

This guide explains how to install HikoGUI for developing HikoGUI itself.
For installing the HikoGUI library as part of an application see the
[hikogui_hello_world](https://github.com/hikogui/hikogui_hello_world)
example application.

Specific installation manuals for each platform and IDE:
 - [Visual Studio Code](docs/build_with_visual_studio_code.md)
 - [MSVC on Windows](docs/build_with_msvc_on_windows.md)

It is currently not recommended to use Visual Studio nor CLion with HikoGUI
due to serious issues, however it used to work:
 - [Visual Studio (Preview)](docs/build_with_visual_studio.md)
 - [gcc on Linux](docs/build_with_gcc_on_linux.md)

Bugs impacting HikoGUI
----------------------

### Bugs in Visual Studio
_Please vote on the following bugs to potentially prioritize fixes by Microsoft_

Components, such as IntelliSense and vcpkgsrv.exe, of Visual Studio will crash
very often due to the many bugs, I've given up on reporting them.

 - <https://developercommunity.visualstudio.com/t/Find-all-references-not-working-CMake/10353435>
 - <https://developercommunity.visualstudio.com/t/I-need-help-fixing-all-the-intellisense/10349228>

We also would like to use non-google unit-testing. However Microsoft plugin
system for unit-test does not work with CMake, therefor only google and boost
unit-testing are supported. We would like to use catch2, but that doesn't work:

 - <https://developercommunity.visualstudio.com/t/CTest-test-adapter-for-Test-Explorer:-ad/640938>

Test Explorer is almost always broken in Visual Studio one of the problems is
the Test Explorer cache that get corrupted after every update of Visual Studio.
This is a feature request to add a button to delete this cache, however this
button does not actually work:

 - <https://developercommunity.visualstudio.com/t/Add-Clear-Test-Explorer-Cache-button-t/877480>


The following are some random MSVC compiler errors that needed to be worked
around:

 - <https://developercommunity.visualstudio.com/t/Failure-to-get-decltype-of-previous-ar/10147771>
 - <https://developercommunity.visualstudio.com/t/C3615-false-positive-when-early-return-f/10395567>

There are also quite a few bugs in the module support of MSVC including crashes
I am trying to make small bug reports for them since Microsoft ignores large
bug reports

 - <https://developercommunity.visualstudio.com/t/C1001:-exporting-a-module/10230789>
 