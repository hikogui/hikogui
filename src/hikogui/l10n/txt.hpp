// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "translation.hpp"
#include "../utility/utility.hpp"
#include "../unicode/module.hpp"
#include "../settings/settings.hpp"
#include "../macros.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

hi_export_module(hikogui.l10n.txt);

namespace hi { inline namespace v1 {

[[nodiscard]] constexpr long long get_first_integer_argument() noexcept
{
    return 0;
}

template<typename First, typename... Rest>
[[nodiscard]] constexpr long long get_first_integer_argument(First const& first, Rest const&...rest) noexcept
{
    if constexpr (std::is_integral_v<First>) {
        return narrow_cast<long long>(first);
    } else {
        return get_first_integer_argument(rest...);
    }
}

/** A localizable message.
 *
 * The translation and formatting of the message is delayed until displaying
 * it to the user. This allows the user to change the language while the
 * application is running.
 */
class txt {
public:
    constexpr txt(txt const&) noexcept = default;
    constexpr txt(txt&&) noexcept = default;
    constexpr txt& operator=(txt const&) noexcept = default;
    constexpr txt& operator=(txt&&) noexcept = default;
    ~txt() = default;
    constexpr txt() noexcept = default;

    [[nodiscard]] constexpr friend bool operator==(txt const& lhs, txt const& rhs) noexcept
    {
        return lhs._msg_id == rhs._msg_id;
    }

    /** Construct a localizable message.
     *
     * It is recommended to use the parentheses form of the constructor so that
     * it will look like a function which is recognized by the `gettext` tool.
     *
     * @param msg_id A English string that is looked up in the translation
     *               database or, when not found, as-is. The msg_id may contain
     *               placeholders using the `std::format` format. Plurality is
     *               based on the first `std::integral` arguments.
     * @param first_arg The first argument passed to `std::format()`.
     * @param args Arguments passed to `std::format()`.
     */
    template<typename... Args>
    txt(std::string msg_id, Args&&...args) noexcept :
        _first_integer_argument(get_first_integer_argument(args...)),
        _msg_id(std::move(msg_id)),
        _args(std::make_format_args(std::forward<Args>(args)...))
    {
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _msg_id.empty();
    }

    /** Check if the message is in use.
     */
    [[nodiscard]] constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    /** Translate and format the message.
     * Find the translation of the message, then format it.
     *
     * @param languages A list of languages to search for translations.
     * @return The translated and formatted message.
     */
    [[nodiscard]] gstring translate(std::vector<language_tag> const& languages = os_settings::language_tags()) const noexcept
    {
        hilet[fmt, language_tag] = ::hi::get_translation(_msg_id, _first_integer_argument, languages);
        hilet msg = std::vformat(fmt, _args);

        // hilet default_attributes = character_attributes{language_tag.expand()};
        // return to_text_with_markup(msg, default_attributes);
        return to_gstring(msg);
    }

    /** Translate and format the message.
     * Find the translation of the message, then format it.
     *
     * @param loc The locale to use when formatting the message.
     * @param languages A list of languages to search for translations.
     * @return The translated and formatted message.
     */
    [[nodiscard]] gstring
    translate(std::locale const& loc, std::vector<language_tag> const& languages = os_settings::language_tags()) const noexcept
    {
        hilet[fmt, language_tag] = ::hi::get_translation(_msg_id, _first_integer_argument, languages);
        hilet msg = std::vformat(loc, fmt, _args);

        // hilet default_attributes = character_attributes{language_tag.expand()};
        // return to_text_with_markup(msg, default_attributes);
        return to_gstring(msg);
    }

    [[nodiscard]] gstring original(std::vector<language_tag> const& languages = os_settings::language_tags()) const noexcept
    {
        hilet msg = std::vformat(_msg_id, _args);

        // hilet default_attributes = character_attributes{language_tag.expand()};
        // return to_text_with_markup(msg, default_attributes);
        return to_gstring(msg);
    }

    explicit operator std::string() const noexcept
    {
        return to_string(this->translate());
    }

private:
    long long _first_integer_argument = 0;
    std::string _msg_id = {};
    std::format_args _args = {};
};

}} // namespace hi::v1

template<typename CharT>
struct std::formatter<hi::txt, CharT> : std::formatter<std::string, CharT> {
    auto format(hi::txt const& t, auto& fc) const
    {
        return std::formatter<std::string, CharT>::format(std::string{t}, fc);
    }
};
