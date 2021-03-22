Change log
==========

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
