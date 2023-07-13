
#pragma once

#define hi_format_argument_check(arg) \
    static_assert( \
        ::std::is_default_constructible_v<std::formatter<std::decay_t<decltype(arg)>>>, \
        "std::format, argument '" #arg "' does not have a specialized std::formatter<>.");

/** A macro to check if the format string and the arguments are valid for std::format.
 *
 * This macro checks if the usage of braces '{' and '}' are correctly used in the format-string.
 * Then it will check if the number of arguments match the number of arguments in the format-string.
 * Lastly it will check if the type for each argument has a valid `std::formatter<>` specialization.
 *
 * This is done in a macro instead of a function, so that the static_asserts will point to the line
 * where the format-string and arguments where defined.
 *
 * @param fmt The `std::format` format-string.
 * @param ... The arguments to be formatted by `std::format`.
 */
#define hi_format_check(fmt, ...) \
    static_assert(::hi::format_count(fmt) != -1, "std::format, Unexpected '{' inside argument-format."); \
    static_assert(::hi::format_count(fmt) != -2, "std::format, Unexpected '}' without corresponding '{'."); \
    static_assert(::hi::format_count(fmt) != -3, "std::format, Missing '}' at end of format string."); \
    static_assert( \
        ::hi::format_count(fmt) == hi_num_va_args(__VA_ARGS__), "std::format, invalid number of arguments for format string."); \
    hi_for_each(hi_format_argument_check, __VA_ARGS__)

#define hi_log(level, fmt, ...) \
    hi_format_check(fmt __VA_OPT__(, ) __VA_ARGS__); \
    ::hi::log_global.add<level, __FILE__, __LINE__, fmt>(__VA_ARGS__)

#define hi_log_debug(fmt, ...) hi_log(::hi::global_state_type::log_debug, fmt __VA_OPT__(, ) __VA_ARGS__)
#define hi_log_info(fmt, ...) hi_log(::hi::global_state_type::log_info, fmt __VA_OPT__(, ) __VA_ARGS__)
#define hi_log_statistics(fmt, ...) hi_log(::hi::global_state_type::log_statistics, fmt __VA_OPT__(, ) __VA_ARGS__)
#define hi_log_trace(fmt, ...) hi_log(::hi::global_state_type::log_trace, fmt __VA_OPT__(, ) __VA_ARGS__)
#define hi_log_audit(fmt, ...) hi_log(::hi::global_state_type::log_audit, fmt __VA_OPT__(, ) __VA_ARGS__)
#define hi_log_warning(fmt, ...) hi_log(::hi::global_state_type::log_warning, fmt __VA_OPT__(, ) __VA_ARGS__)
#define hi_log_error(fmt, ...) hi_log(::hi::global_state_type::log_error, fmt __VA_OPT__(, ) __VA_ARGS__)
#define hi_log_fatal(fmt, ...) \
    hi_log(::hi::global_state_type::log_fatal, fmt __VA_OPT__(, ) __VA_ARGS__); \
    hi_debug_abort()

#define hi_log_info_once(name, fmt, ...) \
    do { \
        if (++::hi::global_counter<name> == 1) { \
            hi_log(::hi::global_state_type::log_info, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } \
    } while (false)

#define hi_log_error_once(name, fmt, ...) \
    do { \
        if (++::hi::global_counter<name> == 1) { \
            hi_log(::hi::global_state_type::log_error, fmt __VA_OPT__(, ) __VA_ARGS__); \
        } \
    } while (false)
