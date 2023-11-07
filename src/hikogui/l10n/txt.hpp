// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "translation.hpp"
#include "../utility/utility.hpp"
#include "../unicode/unicode.hpp"
#include "../settings/settings.hpp"
#include "../macros.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <algorithm>
#include <utility>

hi_export_module(hikogui.l10n.txt);

hi_export namespace hi { inline namespace v1 {

namespace detail {

struct txt_arguments_base {
    virtual ~txt_arguments_base() = default;

    [[nodiscard]] virtual std::unique_ptr<txt_arguments_base> make_unique_copy() const noexcept = 0;
    [[nodiscard]] virtual std::string format(std::locale const &loc, std::string_view fmt) const noexcept = 0;
    [[nodiscard]] virtual bool equal_to(txt_arguments_base const& rhs) const noexcept = 0;
};

template<typename... Types>
struct txt_arguments : txt_arguments_base {
    template<typename... Args>
    constexpr txt_arguments(Args&&...args) : _args(std::forward<Args>(args)...)
    {
    }

    [[nodiscard]] std::unique_ptr<txt_arguments_base> make_unique_copy() const noexcept override
    {
        return std::apply(
            [](auto const&...args) {
                return std::make_unique<txt_arguments>(args...);
            },
            _args);
    }

    [[nodiscard]] std::string format(std::locale const &loc, std::string_view fmt) const noexcept override
    {
        return std::apply(
            [&](auto const&...args) {
                return std::vformat(loc, fmt, std::make_format_args(args...));
            },
            _args);
    }

    [[nodiscard]] bool equal_to(txt_arguments_base const& rhs) const noexcept override
    {
        if (auto *rhs_ = dynamic_cast<txt_arguments const *>(std::addressof(rhs))) {
            return _args == rhs_->_args;
        } else {
            return false;
        }
    }

    std::tuple<Types...> _args;
};

template<typename... Args>
constexpr std::unique_ptr<txt_arguments_base> make_unique_txt_arguments(Args&&...args) noexcept
{
    using txt_arguments_type = txt_arguments<std::decay_t<Args>...>;
    return std::make_unique<txt_arguments_type>(std::forward<Args>(args)...);
}

} // namespace detail

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
hi_export class txt {
public:
    ~txt() = default;

    constexpr txt() noexcept : _first_integer_argument(), _msg_id(), _args(detail::make_unique_txt_arguments()) {}

    txt(txt const& other) noexcept :
        _first_integer_argument(other._first_integer_argument), _msg_id(other._msg_id), _args(other._args->make_unique_copy())
    {
    }

    txt(txt&& other) noexcept
    {
        std::swap(_first_integer_argument, other._first_integer_argument);
        std::swap(_msg_id, other._msg_id);
        std::swap(_args, other._args);
    }

    txt& operator=(txt const& other) noexcept
    {
        if (std::addressof(other) != this) {
            _first_integer_argument = other._first_integer_argument;
            _msg_id = other._msg_id;
            _args = other._args->make_unique_copy();
        }
        return *this;
    }

    txt& operator=(txt&& other) noexcept
    {
        if (std::addressof(other) != this) {
            std::swap(_first_integer_argument, other._first_integer_argument);
            std::swap(_msg_id, other._msg_id);
            std::swap(_args, other._args);
        }
        return *this;
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
        _args(detail::make_unique_txt_arguments(std::forward<Args>(args)...))
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

    [[nodiscard]] constexpr friend bool operator==(txt const& lhs, txt const& rhs) noexcept
    {
        hi_axiom_not_null(lhs._args);
        hi_axiom_not_null(rhs._args);
        return lhs._msg_id == rhs._msg_id and lhs._args->equal_to(*rhs._args);
    }

    /** Translate and format the message.
     * Find the translation of the message, then format it.
     *
     * @param loc The locale to use when formatting the message.
     * @param languages A list of languages to search for translations.
     * @return The translated and formatted message.
     */
    [[nodiscard]] gstring translate(
        std::locale const& loc = os_settings::locale(),
        std::vector<language_tag> const& languages = os_settings::language_tags()) const noexcept
    {
        hi_axiom_not_null(_args);
        hilet[fmt, language_tag] = ::hi::get_translation(_msg_id, _first_integer_argument, languages);
        hilet msg = _args->format(loc, fmt);
        return apply_markup(msg, language_tag);
    }

    /** Translate and format the message.
     * Find the translation of the message, then format it.
     *
     * @param languages A list of languages to search for translations.
     * @return The translated and formatted message.
     */
    [[nodiscard]] gstring translate(std::vector<language_tag> const& languages) const noexcept
    {
        return translate(os_settings::locale(), languages);
    }

    [[nodiscard]] gstring original() const noexcept
    {
        hi_axiom_not_null(_args);
        hilet msg = _args->format(std::locale::classic(), _msg_id);
        return apply_markup(msg, language_tag{"en-US"});
    }

    explicit operator std::string() const noexcept
    {
        return to_string(this->translate());
    }

private:
    long long _first_integer_argument = 0;
    std::string _msg_id = {};
    std::unique_ptr<detail::txt_arguments_base> _args;
};

}} // namespace hi::v1

// XXX #617 MSVC bug does not handle partial specialization in modules.
hi_export template<>
struct std::formatter<hi::txt, char> : std::formatter<std::string, char> {
    auto format(hi::txt const& t, auto& fc) const
    {
        return std::formatter<std::string, char>::format(std::string{t}, fc);
    }
};
