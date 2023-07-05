// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio/module.hpp"
#include "char_maps/module.hpp"
#include "codec/module.hpp"
#include "color/module.hpp"
#include "concurrency/module.hpp"
#include "container/module.hpp"
#include "file/module.hpp"
#include "font/module.hpp"
#include "formula/module.hpp"
#include "geometry/module.hpp"
#include "GFX/module.hpp"
#include "graphic_path/module.hpp"
#include "GUI/module.hpp"
#include "i18n/module.hpp"
#include "image/module.hpp"
#include "l10n/module.hpp"
#include "layout/module.hpp"
#include "memory/module.hpp"
#include "net/module.hpp"
#include "numeric/module.hpp"
#include "observer/module.hpp"
#include "random/module.hpp"
#include "security/module.hpp"
#include "settings/module.hpp"
#include "SIMD/module.hpp"
#include "skeleton/module.hpp"
#include "telemetry/module.hpp"
#include "text/module.hpp"
#include "time/module.hpp"
#include "unicode/module.hpp"
#include "utility/module.hpp"
#include "widgets/module.hpp"

#include "algorithm.hpp"
#include "animator.hpp"
#include "arguments.hpp"
#include "atomic.hpp"
#include "awaitable.hpp"
#include "awaitable_timer.hpp"
#include "bits.hpp"
#include "cache.hpp"
//#include "command_line.hpp"
#include "console.hpp"
// Never include "crt.hpp"
#include "crt_utils.hpp"
#include "datum.hpp"
#include "delayed_format.hpp"
#include "dialog.hpp"
#include "format_check.hpp"
#include "forward_value.hpp"
#include "function_timer.hpp"
#include "functional.hpp"
#include "generator.hpp"
#include "group_ptr.hpp"
#include "huffman.hpp"
#include "indent.hpp"
#include "jsonpath.hpp"
#include "label.hpp"
//#include "lexer.hpp"
//#include "lookahead_iterator.hpp"
#include "loop.hpp"
#include "module.hpp"
#include "operator.hpp"
#include "parse_location.hpp"
#include "pickle.hpp"
#include "placement.hpp"
#include "ranges.hpp"
#include "recursive_iterator.hpp"
#include "reflection.hpp"
#include "scoped_task.hpp"
#include "sip_hash.hpp"
#include "strings.hpp"
#include "task.hpp"
#include "terminate.hpp"
#include "tokenizer.hpp"
#include "when_any.hpp"

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
namespace hi {

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
