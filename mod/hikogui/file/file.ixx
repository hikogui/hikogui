// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;


export module hikogui_file;
export import hikogui_file_access_mode;
export import hikogui_file_file;
export import hikogui_file_file_view;
export import hikogui_file_resource_view;
export import hikogui_file_seek_whence;

export namespace hi {
inline namespace v1 {

/**
\defgroup file File handling utilities.

This module contains file handling utilities:
 - `file` and `file_view` class to read, write and rename files.

File and file-views
-------------------
`file` is a [RAII](https://en.cppreference.com/w/cpp/language/raii)
object holding an handle to an open file.
You can use `access_mode` flags to control how a file is opened:
 - reading, writing,
 - if it should create a new file,
 - truncate the an already existing file,
 - create the directories when creating a file.

A `file_view` is a RAII object holding a memory-mapping of the file.
This object allows easy and fast access to the data in a file, as-if
the file was a `std::span<>` or `std::string_view`.

*/

}}
