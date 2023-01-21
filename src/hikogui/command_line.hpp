// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility/module.hpp"
#include "generator.hpp"

namespace hi::inline v1 {

struct cmdline_short_option {
    char32_t option;
    std::option<std::string> argument;
};

struct cmdline_long_option {
    std::string option;
    std::option<std::string> argument;
};

struct cmdline_executable {
    std::string executable;
};

struct cmdline_non_option {
    std::string argument;
};

using cmdline_option = std::variant<cmline_executable, cmdline_short_option, cmdline_long_option, cmdline_argument>;

/** A POSIX command line parser.
 * The command line tokens passed to this function are the strings passed in via main,
 * or pre-processed by the windows command line pre-processor.
 *
 * Posix commad line argument syntax:
 *  - Single character short-options begin with a '-' or '+'.
 *    '-' options often enable, '+' options disable.
 *  - Multiple short-options may follow a hyphen inside the same token.
 *  - Certain short-options require an argument.
 *  - An short-option and its argument may or may not appear as separate tokens.
 *    For example the '-o' short-option and it argument: `-ofoo` or `-o foo`.
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
    hilet options_with_arguments_ = hi::to_u32string(options_with_arguments);
    auto it = first;

    if (it != last) {
        co_yield cmdln_option::executable{*it};
        ++it;
    }

    char32_t short_option_name = 0;
    for (; it != last; ++it) {
        if (short_option_name) {
            // Add the argument to the option.
            co_yield cmdln_short_option{short_option_name, *it};
            short_option_name = 0;

        } else if (*it == "--") {
            break;

        } else if (it->starts_with("--")) {
            // Long-option
            hilet eq_index = it->find('=');
            if (eq_index == std::string::npos) {
                // Long-option without argument
                co_yield cmdln_long_option{it->substr(2), {}};

            } else {
                // Long-option with argument in same token.
                hi_assert(eq_index >= 2);
                hilet name_length = eq_index - 2;
                co_yield cmdln_long_option(it->substr(2, name_length), it->substr(eq_index + 1));
            }

        } else if (it.front() == '-' or it.front() == '+') {
            // List of short-options.
            // Short options are processed as UTF-32 units.
            hilet token = to_u32string(*it);
            hilet first_c = std::next(begin(token));
            hilet last_c = end(token);
            negative = it.front() == '+';

            for (auto jt = first_c; jt != last_c; ++jt) {
                hilet c = *jt;
                auto name = hi::to_string(std::u32string(1, c));

                if (options_with_arguments_.find(c) == std::u32string::npos) {
                    // Option without argument
                    co_yield cmdln_short_option(c, {});

                } else if (std::next(jt) == last) {
                    // Option with the argument in the next token.
                    short_option_name = c;

                } else {
                    // Option with argument, where the argument is inside this token
                    auto argument = hi::to_string(std::u32string(std::next(jt), last_c));
                    co_yield cmdln_short_option(c, std::move(argument));
                    break;
                }
            }

        } else {
            // Anything not looking like an option is a non-option
            co_yield cmdln_non_option{*it};
        }
    }

    // All tokens after double hyphen '--' are non-options.
    for (; it != last; ++it) {
        co_yield cmdln_non_option{*it};
    }

    if (short_option_name) {
        throw parse_error("Missing argument for option -{}", short_option_name);
    }
}

class command_line_option {
public:
    char32_t short_option;
    std::string long_option;
    std::string argument_name;
    std::string description;
    hi::notifier<void(std::string_view argument)> notifier;

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
    constexpr command_line_option(std::string_view option_help)
    {
        auto it = begin(option_help);
        hilet last = end(option_help);
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
#if HI_OPERATING_SYSTEM == HI_OS_WINDOWS
    void parse();
#else
    void parse(int argc, char **argv);
#endif

    template<typename... Args>
    command_line_option &add_option(Args &&...args) noexcept
    {
        return _options.emplace(std::forward<Args>(args)...);
    }

private:
    std::vector<command_line_option> _options = {};
};

} // namespace hi::inline v1
