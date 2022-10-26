// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "language.hpp"
#include "translation.hpp"
#include "../forward_value.hpp"
#include "../cast.hpp"
#include "../os_settings.hpp"
#include <memory>
#include <string>
#include <string_view>
#include <tuple>

namespace hi::inline v1 {
namespace detail {
class translate_args_base {
public:
    virtual ~translate_args_base() {}
    translate_args_base() = default;
    translate_args_base(translate_args_base const&) = default;
    translate_args_base(translate_args_base&&) = default;
    translate_args_base& operator=(translate_args_base const&) = default;
    translate_args_base& operator=(translate_args_base&&) = default;

    /** Format text from the arguments and the given format string.
     * @param fmt The format string.
     */
    [[nodiscard]] virtual std::string format(std::string_view fmt) const noexcept = 0;

    /** Format text from the arguments and the given format string.
     * @param loc The locale to use when formatting.
     * @param fmt The format string.
     */
    [[nodiscard]] virtual std::string format(std::locale const& loc, std::string_view fmt) const noexcept = 0;

    /** The numeric value of the first numeric argument.
     * @return The numeric value of the first numeric argument or zero.
     */
    [[nodiscard]] virtual long long n() const noexcept = 0;

    /** Make a unique copy of the arguments.
     */
    [[nodiscard]] virtual std::unique_ptr<translate_args_base> unique_copy() const noexcept = 0;

    [[nodiscard]] virtual bool equal_to(translate_args_base const& rhs) const noexcept = 0;

    [[nodiscard]] bool friend operator==(translate_args_base const& lhs, translate_args_base const& rhs) noexcept
    {
        return lhs.equal_to(rhs);
    }
};

/** Delayed formatting.
 * This class will capture all the arguments so that it may be passed
 * to another thread. Then call the function operator to do the actual formatting.
 */
template<typename... Values>
class translate_args : public translate_args_base {
public:
    translate_args(translate_args&&) noexcept = default;
    translate_args(translate_args const&) noexcept = default;
    translate_args& operator=(translate_args&&) noexcept = default;
    translate_args& operator=(translate_args const&) noexcept = default;

    /** Construct a translate arguments.
     *
     * All arguments are passed by forwarding-references so that values can be
     * moved into the storage of the translate object.
     *
     * Arguments passed by reference will be copied. Arguments passed by std::string_view
     * or std::span will be copied into a std::string or std::vector.
     *
     * Literal strings will not be copied, instead a pointer is taken.
     *
     * @param args The parameters to std::format excluding format string and locale.
     */
    template<typename... Args>
    translate_args(Args const&...args) noexcept : _values(args...)
    {
    }

    [[nodiscard]] std::unique_ptr<translate_args_base> unique_copy() const noexcept override
    {
        return std::make_unique<translate_args>(*this);
    }

    [[nodiscard]] virtual bool equal_to(translate_args_base const& rhs) const noexcept override
    {
        if (auto *rhs_ = dynamic_cast<translate_args const *>(&rhs)) {
            return _values == rhs_->_values;
        } else {
            return false;
        }
    }

    [[nodiscard]] std::string format(std::string_view fmt) const noexcept override
    {
        return std::apply(format_wrapper<Values const&...>, std::tuple_cat(std::tuple{fmt}, _values));
    }

    [[nodiscard]] std::string format(std::locale const& loc, std::string_view fmt) const noexcept override
    {
        return std::apply(format_locale_wrapper<Values const&...>, std::tuple_cat(std::tuple{loc, fmt}, _values));
    }

    template<std::size_t I>
    [[nodiscard]] long long n_recurse() const noexcept
    {
        if constexpr (I < sizeof...(Values)) {
            if constexpr (std::is_integral_v<decltype(std::get<I>(_values))>) {
                return narrow_cast<long long>(std::get<I>(_values));
            } else {
                return n_recurse<I + 1>();
            }
        } else {
            return 0;
        }
    }

    [[nodiscard]] long long n() const noexcept override
    {
        return n_recurse<0>();
    }

private:
    std::tuple<Values...> _values;

    template<typename... Args>
    static std::string format_wrapper(std::string_view fmt, Args const&...args)
    {
        return std::vformat(fmt, std::make_format_args(args...));
    }

    template<typename... Args>
    static std::string format_locale_wrapper(std::locale const& loc, std::string_view fmt, Args const&...args)
    {
        return std::vformat(loc, fmt, std::make_format_args(args...));
    }
};

template<typename... Args>
translate_args(Args&&...) -> translate_args<forward_value_t<Args>...>;

} // namespace detail

/** A localizable message.
 *
 * The translation and formatting of the message is delayed until displaying
 * it to the user. This allows the user to change the language while the
 * application is running.
 */
class translate {
public:
    ~translate() = default;

    /** Construct an empty message.
     */
    constexpr translate() noexcept : _msg_id(), _args(nullptr), _has_args(false) {}

    constexpr translate(translate&& other) noexcept :
        _msg_id(std::move(other._msg_id)), _args(nullptr), _has_args(other._has_args)
    {
        if (_has_args) {
            _args = std::move(other._args);
        }
    }

    constexpr translate& operator=(translate&& other) noexcept
    {
        _msg_id = std::move(other._msg_id);
        _has_args = other._has_args;
        if (_has_args) {
            _args = std::move(other._args);
        }
        return *this;
    }

    constexpr translate(translate const& other) noexcept : _msg_id(other._msg_id), _args(nullptr), _has_args(other._has_args)
    {
        if (_has_args) {
            _args = other._args->unique_copy();
        }
    }

    constexpr translate& operator=(translate const& other) noexcept
    {
        _msg_id = other._msg_id;
        _has_args = other._has_args;
        if (_has_args) {
            _args = other._args->unique_copy();
        }
        return *this;
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

    /** Construct a localizable message.
     *
     * It is recommended to use the parentheses form of the constructor so that
     * it will look like a function which is recognized by the `gettext` tool.
     *
     * @param msg_id A English string that is looked up in the translation
     *               database or, when not found, as-is. The msg_id may contain
     *               placeholders using the `std::format` format. Plurality is
     *               based on the first `std::integral` arguments.
     */
    constexpr translate(std::string_view msg_id) noexcept : _msg_id(msg_id), _args(nullptr), _has_args(false) {}

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
    template<typename FirstArg, typename... Args>
    translate(std::string_view msg_id, FirstArg const& first_arg, Args const&...args) noexcept :
        _msg_id(msg_id),
        _args(std::make_unique<detail::translate_args<forward_value_t<FirstArg>, forward_value_t<Args>...>>(first_arg, args...)),
        _has_args(true)
    {
    }

    /** Translate and format the message.
     * Find the translation of the message, then format it.
     *
     * @param languages A list of languages to search for translations.
     * @return The translated and formatted message.
     */
    [[nodiscard]] std::string operator()(std::vector<language *> const& languages = os_settings::languages()) const noexcept
    {
        if (_has_args) {
            hilet fmt = ::hi::get_translation(_msg_id, _args->n(), languages);
            return _args->format(fmt);
        } else {
            return std::string{::hi::get_translation(_msg_id, 0, languages)};
        }
    }

    /** Translate and format the message.
     * Find the translation of the message, then format it.
     *
     * @param loc The locale to use when formatting the message.
     * @param languages A list of languages to search for translations.
     * @return The translated and formatted message.
     */
    [[nodiscard]] std::string
    operator()(std::locale const& loc, std::vector<language *> const& languages = os_settings::languages()) const noexcept
    {
        if (_args) {
            hilet fmt = ::hi::get_translation(_msg_id, _args->n(), languages);
            return _args->format(loc, fmt);
        } else {
            return std::string{::hi::get_translation(_msg_id, 0, languages)};
        }
    }

    /** Compare two localizable messages.
     *
     * @param lhs A localizable message.
     * @param rhs A localizable message.
     * @return True if both messages are equal.
     */
    [[nodiscard]] constexpr friend bool operator==(translate const& lhs, translate const& rhs) noexcept
    {
        if (lhs._has_args != rhs._has_args) {
            return false;
        }

        if (lhs._has_args and *lhs._args != *rhs._args) {
            return false;
        }

        return lhs._msg_id == rhs._msg_id;
    }

private:
    std::string _msg_id;
    std::unique_ptr<detail::translate_args_base> _args;
    // Technically we could check _args for nullptr. However to get this working
    // with constexpr constructor we need a way to disable the std::unique_ptr.
    bool _has_args;
};

using tr = translate;

} // namespace hi::inline v1
