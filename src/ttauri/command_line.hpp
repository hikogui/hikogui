
#include "coroutine"
#include "os_detect.hpp"

#pragma once

namespace tt {
namespace detail {

struct command_line_parsed_option {
    /** Name of the option.
     * The string is not empty when is_option is true.
     * The string is empty when is_option is false.
     * With a short-option the string contains a single unicode code-point, but may
     * be multiple UTF-8 code units.
     */
    std::string name = {};

    /** Argument for the option.
     * For a short-option with required argument the string is not empty.
     * For a long-option with an argument the string may be empty.
     * For a long-option without an argument the string is empty.
     * For a non-option
     */
    std::string argument = {};

    /** This is an option.
     * When true this is an option, when false this is a non-option.
     */
    bool is_option = false;

    /** This is a long option.
     * When true this is a long-option, when false this is a short-option.
     * Should be set to false when this is a non-option.
     */
    bool is_long = false;

    /** Option has an argument.
     * When this is an option:
     *   True when the option has an argument.
     * WHen this is a non-option:
     *   False when this is the name of the executable
     *   True if this is a non-option.
     */
    bool has_argument = false;

    /** Factory for a short option with required argument.
     * The argument can not be empty.
     *
     * @param name A single character short-option name
     * @param argument The required argument.
     */
    [[nodiscard]] static constexpr command_line_parsed_option short_option(char32_t name, std::string argument) noexcept
    {
        tt_axiom(!argument.empty());

        auto r = command_line_parsed_option{};
        r.name = tt::to_string(std::u32string{1, name});
        r.argument = std::move(argument);
        r.is_option = true;
        r.is_long = false;
        r.has_argument = true;
        return r;
    }

    /** Factory for a short option.
     *
     * @param name A single character short-option name
     */
    [[nodiscard]] static constexpr command_line_parsed_option short_option(char32_t name) noexcept
    {
        auto r = command_line_parsed_option{};
        r.name = tt::to_string(std::u32string{1, c});
        r.is_option = true;
        r.is_long = false;
        r.has_argument = false;
        return r;
    }

    [[nodiscard]] static constexpr command_line_parsed_option long_option(std::string name, std::string argument) noexcept
    {
        auto r = command_line_parsed_option{};
        r.name = std::move(name);
        r.argument = std::move(argument);
        r.is_option = true;
        r.is_long = true;
        r.has_argument = true;
        return r;
    }
    
    [[nodiscard]] static constexpr command_line_parsed_option long_option(std::string name) noexcept
    {
        auto r = command_line_parsed_option{};
        r.name = std::move(name);
        r.is_option = true;
        r.is_long = true;
        r.has_argument = false;
        return r;
    }
    
    [[nodiscard]] static constexpr command_line_parsed_option non_option(std::string argument) noexcept
    {
        auto r = command_line_parsed_option{};
        r.argument = std::move(argument);
        r.is_option = false;
        r.is_long = false;
        r.has_argument = true;
        return r;
    }
    
    [[nodiscard]] static constexpr command_line_parsed_option executable(std::string name) noexcept
    {
        auto r = command_line_parsed_option{};
        r.name = std::move(name);
        r.is_option = false;
        r.is_long = false;
        r.has_argument = false;
        return r;
    }
};

/** A POSIX command line parser.
 * The command line tokens passed to this function are the strings passed in via main,
 * or pre-processed by the windows command line pre-processor.
 *
 * Posix commad line argument syntax:
 *  - Single character short-options begin with a '-'.
 *  - Multiple short-options may follow a hyphen inside the same token.
 *  - Certain short-options require an argument.
 *  - An short-option and its argument may or may not appear as separate tokens.
 *    For example the '-o' short-option and it argument: #-ofoo# or #-o foo#.
 *    Any character may be used in the argument, including a single hyphen, which
 *    by convention is either the stdin or stdout stream.
 *  - A long-options starts with a '--' and are followed by
 *    a string of characters. Optionally an long-option is followed by a
 *    '=' character and an argument in the same token.
 *  - A token with just a double hyphen '--' terminates option parsing. All tokens
 *    after the double hyphen are treated as non-option arguments.
 *  - Everything else is a non-option argument.
 *
 * This function will properly handle UTF-8 encoded strings. Including single character
 * options where the character is represented with multiple UTF-8 code units.
 * 
 * @tparam It A forward iterator
 * @param first Iterator to the first command line token
 * @param last Iterator one beyond the last command line token
 * @param options_with_arguments A list of single character options that accept an argument.
 */
template<typename It>
generator<cmdln_option> command_line_parser(It first, It last, std::string_view options_with_arguments)
{
    ttlet options_with_arguments_ = tt::to_u32string(options_with_arguments);
    auto it = first;

    if (it != last) {
        co_yield cmdln_option::executable(*it);
        ++it;
    }

    char32_t short_option_name = 0;
    for (; it != last; ++it) {
        if (short_option_name) {
            // Add the argument to the option.
            co_yield cmdln_option::short_option(short_option_name, *it);
            short_option_name = 0;

        } else if (*it == "--") {
            break;

        } else if (it->starts_with("--")) {
            // Long-option
            ttlet eq_index = it->find('=');
            if (eq_index == std::string::npos) {
                // Long-option without argument
                co_yield cmdln_option::long_option(it->substr(2));

            } else {
                // Long-option with argument in same token.
                tt_axiom(eq_index >= 2);
                ttlet name_length = eq_index - 2;
                co_yield cmdln_option::long_option(it->substr(2, name_length), it->substr(eq_index + 1));
            }

        } else if (it.front() == '-' || it.front() == '+') {
            // List of short-options.
            // Short options are processed as UTF-32 units.
            ttlet token = to_u32string(*it);
            ttlet first_c = std::next(std::begin(token));
            ttlet last_c = std::end(token);
            negative = it.front() == '+';

            for (auto jt = first_c; jt != last_c; ++jt) {
                ttlet c = *jt;
                auto name = tt::to_string(std::u32string(1, c));

                if (options_with_arguments_.find(c) == std::u32string::npos) {
                    // Option without argument
                    co_yield cmdln_option::short_option(c);

                } else if (std::next(jt) == last) {
                    // Option with the argument in the next token.
                    short_option_name = c;

                } else {
                    // Option with argument, where the argument is inside this token
                    auto argument = tt::to_string(std::u32string(std::next(jt), last_c));
                    co_yield cmdln_option::short_option(c, std::move(argument));
                    break;
                }
            }

        } else {
            // Anything not looking like an option is a non-option
            co_yield cmdln_option::non_option(*it);
        }
    }

    // All tokens after double hyphen '--' are non-options.
    for (;it != last; ++it) {
        co_yield cmdln_option::non_option(*it);
    }

    if (short_option_name) {
        throw parse_error("Missing argument for option -{}", short_option_name);
    }
}

}

class command_line_option {
public:
    char32_t short_option;
    std::string long_option;
    std::string argument_name;
    std::string description;
    tt::notifier<void(std::string_view argument)> notifier;

    /**
     *
     * Syntax:
     *    
     *    option_help = [ short_option ',' ] long_option ' ' description
     *
     *    short_option = '-' /[^-=]/
     *
     *    long_option = '--' name [ '=' name ]
     *
     *    description = /.+/
     *
     *    name = /[^=]+/
     *
     * Example:
     *     command_line_option("-f,--foo=filename Set filename for foo.", [](auto filename) {
     *         global_foo = filename;
     *     });
     */
    constexpr command_line_option(std::string_view option_help) {
        auto it = std::begin(option_help);
        ttlet last = std::end(option_help);




    }

    static char32_t parse_short_option(std::string_view::iterator &it, std::string_view::iterator last)
    {
        if (it == last || *it != '-') {
            throw parse_error("Expecting '-'");
        }

        ++it;

        if (it == last) {
            throw parse_error("Missing character after '-'");
        }
        if (*it == '-') {
            return 0;
        }
    }
};

/** Command line parser.
 */
class command_line {
public:
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
    void parse();
#else
    void parse(int argc, char **argv);
#endif

    template<typename... Args>
    command_line_option &add_option(Args &&... args) noexcept {
        return _options.emplace(std::forward<Args>(args)...);
    }

private:
    std::vector<command_line_option> _options = {};
};


}

