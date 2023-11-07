Open bugs impacting HikoGUI
===========================
There are a lot of bugs impacting HikoGUI.
You may want to 'like' these bugs to hopefully get them resolved quicker.

Bugs in Visual Studio
---------------------
_Please vote on the following bugs to potentially prioritize fixes by Microsoft_

Components, such as IntelliSense and vcpkgsrv.exe, of Visual Studio will crash
very often due to the many bugs, I've given up on reporting them.

 - <https://developercommunity.visualstudio.com/t/Find-all-references-not-working-CMake/10353435>
 - <https://developercommunity.visualstudio.com/t/I-need-help-fixing-all-the-intellisense/10349228>

We also would like to use non-google unit-testing. However Microsoft plugin
system for unit-test does not work with CMake, therefor only google and boost
unit-testing are supported. We would like to use catch2, but that doesn't work:

 - <https://developercommunity.visualstudio.com/t/CTest-test-adapter-for-Test-Explorer:-ad/640938>

Alignof does not work in natvis:

 - <https://developercommunity.visualstudio.com/t/alignof-and-__alignof-in-watch-window-an/1012651>

Annoying that you can not permanently turn off outlining:
 - <https://developercommunity.visualstudio.com/t/Cannot-permanently-disable-outlining/408289>

If Visual Studio crashes the options you have changed hours ago are not saved:
 - <https://developercommunity.visualstudio.com/t/Automatically-save-visual-studio-configu/1531962>

Missing x64 version of the development powershell:
 - <https://developercommunity.visualstudio.com/t/The-Developer-PowerShell-for-VS-2022-sho/1568773>

Test Explorer is almost always broken in Visual Studio one of the problems is
the Test Explorer cache that get corrupted after every update of Visual Studio.
This is a feature request to add a button to delete this cache, however this
button does not actually work:

 - <https://developercommunity.visualstudio.com/t/Add-Clear-Test-Explorer-Cache-button-t/877480>

No git submodule support
 - <https://developercommunity.visualstudio.com/t/Full-Git-submodule-support/351549>

The feedback.exe that is used to report bugs, sometimes crashes:
 - <https://developercommunity.visualstudio.com/t/feedbackexe-crash-when-selecting-My-fe/10308420>

Feature request to have local variables have a different color:
 - <https://developercommunity.visualstudio.com/t/Add-C-Local-Constants-to-Options--E/10335703>

Request for changes to sticky scroll:
 - <https://developercommunity.visualstudio.com/t/Ideas-for-improvement-of-sticky-scroll-f/10337490>

Bugs in CLion
-------------
After CLion finishing indexing the project it becomes extremely slow, simply grinds to a halt.
Someone has looked at this and you can disable the indexing server to make it perform well
again, however that means no more context aware syntax highlighting, searching, refactoring, etc.

Bugs in MSVC
------------
These are open bugs in MSVC that were/are encountered with HikoGUI.

`constinit` of `std::optional<value_type>` value does not work if the
constructor of `value_type` is not `constexpr`.
 - <https://developercommunity.visualstudio.com/t/C:-constinit-for-an-optional-fails-if/1406069>

ICE on incomplete code:
 - <https://developercommunity.visualstudio.com/t/INTERNAL-COMPILER-ERROR-on-broken-code/1623328>

ICE on try-catch in constexpr function when using C++20 modules:
 - <https://developercommunity.visualstudio.com/t/Internal-compiler-error-when-using-a-try/1638307>

utf-8 string literal incorrectly handle byte escape codes vs character escape codes
 - <https://developercommunity.visualstudio.com/t/escape-sequences-in-unicode-string-liter/260684>

Failure on `decltype()` on a `auto` function argument.
 - <https://developercommunity.visualstudio.com/t/Failure-to-get-decltype-of-previous-ar/10147771>

Unknown ICE on creating a large module file.
 - <https://developercommunity.visualstudio.com/t/C1001:-exporting-a-module/10230789>

False positive C3615 when early return of non-first switch label.
 - <https://developercommunity.visualstudio.com/t/C3615-false-positive-when-early-return-f/10395567>

Can't use `std::expected` with `constexpr` function in a C++20 modules.
 - <https://developercommunity.visualstudio.com/t/C1907-modules-constexpr-and-std::expect/10501314>

Template deduction guide does not "export" types and function from included headers.
Possible C++ language defect.
 - <https://developercommunity.visualstudio.com/t/c20-module-template-deduction-guide-st/10501413>

Spaceship operator requires <compare> to be included even if the spaceship operator is not
used (implicitly converted less-than operator) in a C++20 module.
 - <https://developercommunity.visualstudio.com/t/C1001-C20-modules-that-uses-spaceship/10501441>

Templates do not export user-defined literals in C++20 modules.
 - <https://developercommunity.visualstudio.com/t/C3688-C20-modules-user-defined-litera/10503288>

ICE when using auto arguments of a member function in a templated class that is instantiated
in a second C++20 module.
 - <https://developercommunity.visualstudio.com/t/C1001-Crash-auto-argument-on-member-func/10503465>

Partial template specialization in C++20 modules do not work.
 - <https://developercommunity.visualstudio.com/t/Partial-specialization-of-std::formatter/10506520>

Unknown ICE when specializing `std::char_traits` for `grapheme` in C++20 module.
THIS IS CURRENTLY BLOCKING FURTHER DEVELOPMENT IN USING MODULES.
 - <https://developercommunity.visualstudio.com/t/C1001-template-specialization-of-char_tr/10508726>
