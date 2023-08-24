// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "audio/audio.hpp"
#include "algorithm/module.hpp"
#include "char_maps/module.hpp"
#include "codec/codec.hpp"
#include "color/module.hpp"
#include "concurrency/concurrency.hpp"
#include "console/console.hpp"
#include "container/module.hpp"
#include "coroutine/module.hpp"
#include "crt/crt.hpp"
#include "file/file.hpp"
#include "font/module.hpp"
#include "formula/formula.hpp"
#include "geometry/module.hpp"
#include "GFX/module.hpp"
#include "graphic_path/graphic_path.hpp"
#include "GUI/module.hpp"
#include "i18n/i18n.hpp"
#include "image/module.hpp"
#include "l10n/l10n.hpp"
#include "dispatch/dispatch.hpp"
#include "layout/module.hpp"
#include "memory/memory.hpp"
#include "net/module.hpp"
#include "numeric/module.hpp"
#include "observer/module.hpp"
#include "parser/parser.hpp"
#include "path/path.hpp"
#include "random/random.hpp"
#include "security/module.hpp"
#include "settings/settings.hpp"
#include "SIMD/module.hpp"
#include "skeleton/module.hpp"
#include "telemetry/module.hpp"
#include "text/module.hpp"
#include "time/module.hpp"
#include "unicode/unicode.hpp"
#include "utility/utility.hpp"
#include "widgets/module.hpp"

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
