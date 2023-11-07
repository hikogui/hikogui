// Copyright Take Vos 2022, 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;


export module hikogui_char_maps;
export import hikogui_char_maps_ascii;
export import hikogui_char_maps_char_converter;
export import hikogui_char_maps_cp_1252;
export import hikogui_char_maps_iso_8859_1;
export import hikogui_char_maps_to_string;
export import hikogui_char_maps_utf_16;
export import hikogui_char_maps_utf_32;
export import hikogui_char_maps_utf_8;

export namespace hi {
inline namespace v1 {
/**
\defgroup char_maps Character Maps

These templates are uses to convert between different character encodings.

You can also convert between the same character encoding, this can be done to
'repair' invalid encodings. For example when converting from ASCII to ASCII any
characters in the original string above 127 will be replaced with a
question mark '?' after conversion. For UTF-8 to UTF-8 invalid encodings may be interpreted
as CP-1252 and correctly converted to the unicode equivalent, or the replacement
character U+fffd.

Here is an example to convert a string from ISO-8859-1 to UTF-8:

```
std::string latin1_to_utf8(std::string_view str)
{
    return hi::char_converter<"iso-8859-1", "utf-8">{}(str);
}
```

Currently supported are:

 - ASCII
 - CP-1252 / Windows-1252
 - ISO-8859-1 / Latin-1
 - Unicode UTF-8
 - Unicode UTF-16
 - Unicode UTF-32

It is possible to add character encodings by specializing `char_map<>` in your application.

*/
}}