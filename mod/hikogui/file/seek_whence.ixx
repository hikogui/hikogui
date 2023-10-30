

module;
#include "../macros.hpp"


export module hikogui_file_seek_whence;

export namespace hi { inline namespace v1 {

/** The position in the file to seek from.
 * @ingroup file
 */
export enum class seek_whence {
    begin, ///< Start from the beginning of the file.
    current, ///< Continue from the current position.
    end ///< Start from the end of the file.
};

}}
