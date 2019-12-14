
#include "TTauri/Foundation/os_detect.hpp"

namespace TTauri {

/** Abort the application.
 * @param source_file __FILE__
 * @param source_line __LINE__
 * @param message Message to display.
 */
[[noreturn]] void _ttauri_abort(char const *source_file, int soure_line, char const *message);

/** Abort the application.
 * @param source_file __FILE__
 * @param source_line __LINE__
 * @param message Message to display.
 */
[[noreturn]] no_inline void _ttauri_abort(char const *source_file, int soure_line);

/** Abort the application.
 * @param source_file __FILE__
 * @param source_line __LINE__
 * @param message Message to display.
 * @param arg1 First argument to formatter
 * @param args Rest arguments to formatter
 */
template<typename Arg1, typename... Args>
[[noreturn]] no_inline _ttauri_abort(char const *source_file, source_line, char const *message, Arg1 &&arg1, Args &&... args)
{
    std::string formatted_message = fmt::format(message, std::forward<Arg1>(arg1), std::forward<Args>(args)...);
    _ttauri_abort(source_file, source_line, formatted_message.data());
}

#define ttauri_abort(...) _ttauri_abort(__FILE__, __LINE__, __VA_ARGS__)

}
