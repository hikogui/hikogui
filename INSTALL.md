Install the HikoGUI development environment
===========================================

This guide explains how to install HikoGUI for developing HikoGUI itself.
For installing the HikoGUI library as part of an application see the
[hikogui_hello_world](https://github.com/hikogui/hikogui_hello_world)
example application.

Specific installation manuals for each platform and IDE:
 - [Visual Studio (Preview)](docs/build_with_visual_studio.md)
 - [Visual Studio (Preview) + vcpkg](docs/build_with_visual_studio_and_vcpkg.md)

Bugs impacting HikoGUI
----------------------

### Bugs in Visual Studio
_Please vote on the following bugs to potentially prioritize fixes by Microsoft_

Components, such as IntelliSense and vcpkgsrv.exe, of Visual Studio will crash very often due to
the following bug:

 - <https://developercommunity.visualstudio.com/t/Intellisense-and-vcpkgsrvexe-keeps-cras/10159818>

The following bugs cause thousands of false positive warnings and errors from IntelliSense. You should
set Error List window to show "Build Only" to ignore IntelliSense.

 - <https://developercommunity.visualstudio.com/t/Intellisense-fails-to-keep-track-of-type/1614419>
 - <https://developercommunity.visualstudio.com/t/Intellisense-false-positive-E3244-templa/1614488>
 - <https://developercommunity.visualstudio.com/t/Intellisense-E0028-expression-must-have/1614411>

We also would like to use non-google unit-testing. However Microsoft does plugin system for unit-test does
not work with CMake, therefor only google and boost unit-testing are supported. We would like to use
catch2, but that doesn't work:

 - <https://developercommunity.visualstudio.com/t/CTest-test-adapter-for-Test-Explorer:-ad/640938>

Test Explorer is almost always broken in Visual Studio one of the problems is the Test Explorer
cache that get corrupted. This is a feature request to add a button to delete this cache:

 - <https://developercommunity.visualstudio.com/t/Add-Clear-Test-Explorer-Cache-button-t/877480>

The following are some random MSVC compiler errors that needed to be worked around:

 - <https://developercommunity.visualstudio.com/t/C2131-templated-static-constexpr-member/1492745>
 - <https://developercommunity.visualstudio.com/t/Failure-to-get-decltype-of-previous-ar/10147771>
