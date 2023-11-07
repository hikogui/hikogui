// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;


export module hikogui;
export import hikogui_GFX;
export import hikogui_GUI;
export import hikogui_SIMD;
export import hikogui_algorithm;
export import hikogui_audio;
export import hikogui_char_maps;
export import hikogui_codec;
export import hikogui_color;
export import hikogui_concurrency;
export import hikogui_console;
export import hikogui_container;
export import hikogui_coroutine;
export import hikogui_crt;
export import hikogui_dispatch;
export import hikogui_file;
export import hikogui_font;
export import hikogui_formula;
export import hikogui_geometry;
export import hikogui_graphic_path;
export import hikogui_i18n;
export import hikogui_image;
export import hikogui_l10n;
export import hikogui_layout;
export import hikogui_memory;
export import hikogui_net;
export import hikogui_numeric;
export import hikogui_observer;
export import hikogui_parser;
export import hikogui_path;
export import hikogui_random;
export import hikogui_security;
export import hikogui_settings;
export import hikogui_skeleton;
export import hikogui_telemetry;
export import hikogui_text;
export import hikogui_time;
export import hikogui_unicode;
export import hikogui_utility;
export import hikogui_widgets;
export import hikogui_win32;

//#include "command_line.hpp"
// Never include "crt.hpp"

/** @file module.hpp
* 
* 
* Dependencies:
*  - utility: -
*  - SIMD: utility
*  - geometry: SIMD, utility, concurrency
*  - color: geometry, SIMD, utility
*  - image: geometry, SIMD, color, utility
*  - char_maps: utility
* 
*/

/** @namespace hi The HikoGUI namespace.
 */
export namespace hi {

/** @namespace hi::v1 The HikoGUI API version 1.
 */
inline namespace v1 {

} // namespace v1
} // namespace hi

/** @namespace v1 DOXYGEN BUG
 * @brief Doxygen can't handle `namespace hi::inline v1 {}` syntax.
 * All files should be changed to use old-style inline namespace syntax.
 */
namespace v1{}
