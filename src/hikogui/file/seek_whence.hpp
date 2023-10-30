

#pragma once

#include "../macros.hpp"

hi_export_module(hikogui.file.seek_whence);

hi_export namespace hi { inline namespace v1 {

/** The position in the file to seek from.
 * @ingroup file
 */
hi_export enum class seek_whence {
    begin, ///< Start from the beginning of the file.
    current, ///< Continue from the current position.
    end ///< Start from the end of the file.
};

}}
