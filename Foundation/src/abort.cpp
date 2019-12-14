
#include "TTauri/Foundation/abort.hpp"
#include "TTauri/Foundation/os_detect.hpp"

namespace TTauri {

/** Abort the application.
 * @param source_file __FILE__
 * @param source_line __LINE__
 * @param message Message to display.
 */
[[noreturn]] void _ttauri_abort(char const *source_file, int soure_line, char const *message)
{
    if (debugger_is_present()) {
        debugger_log("{}:{} {}", source_file, source_line, message);
        debugger_break();
    } else {
        debugger_dialogue("Aborting", "{}:{} {}", source_file, source_line, message);
    }


    std::abort();
}

/** Abort the application.
 * @param source_file __FILE__
 * @param source_line __LINE__
 * @param message Message to display.
 */

[[noreturn]] void _ttauri_abort(char const *source_file, int soure_line)
{
    _ttauri_abort(source_file, source_line, "<no message>");
}

}
