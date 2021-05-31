// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "strings.hpp"
#include "observable.hpp"
#include "icon.hpp"
#include "l10n.hpp"
#include <string>
#include <type_traits>
#include <memory>

namespace tt {
namespace detail {

class label_arguments_base {
public:
    label_arguments_base() noexcept = default;
    virtual ~label_arguments_base() = default;
    label_arguments_base(label_arguments_base const &) = delete;
    label_arguments_base(label_arguments_base &&) = delete;
    label_arguments_base &operator=(label_arguments_base const &) = delete;
    label_arguments_base &operator=(label_arguments_base &&) = delete;

    [[nodiscard]] virtual std::string format(std::string_view fmt) const noexcept = 0;
    [[nodiscard]] virtual std::unique_ptr<label_arguments_base> make_unique_copy() const noexcept = 0;
    [[nodiscard]] virtual bool eq(label_arguments_base &other) const noexcept = 0;
};

template<typename... Types>
class label_arguments : public label_arguments_base {
public:
    template<typename... Args>
    label_arguments(Args &&... args) noexcept : _args(std::forward<Args>(args)...) {}

    [[nodiscard]] std::string format(std::string_view fmt) const noexcept override
    {
        return std::apply(
            [&fmt](auto const &... args) {
                return fmt::format(fmt, args...);
            },
            _args);
    }

    [[nodiscard]] std::unique_ptr<label_arguments_base> make_unique_copy() const noexcept override
    {
        return std::apply(
            [](auto const &... args) {
                return std::make_unique<label_arguments<Types...>>(args...);
            },
            _args);
    }

    [[nodiscard]] bool eq(label_arguments_base &other) const noexcept override
    {
        auto *other_ = dynamic_cast<label_arguments *>(&other);
        if (other_ == nullptr) {
            return false;
        }

        return _args == other_->_args;
    }

private:
    std::tuple<Types...> _args;
};

} // namespace detail

/** A localized text + icon label.
 */
class label {
public:
    template<typename... Args>
    label(tt::icon icon, l10n fmt, Args &&... args) noexcept :
        _icon(std::move(icon)),
        _fmt(std::move(fmt)),
        _args(std::make_unique<detail::label_arguments<std::remove_cvref_t<Args>...>>(std::forward<Args>(args)...))
    {
    }

    template<typename... Args>
    label(l10n fmt, Args &&... args) noexcept : label(tt::icon{}, std::move(fmt), std::forward<Args>(args)...)
    {
    }

    label(tt::icon icon) noexcept : label(std::move(icon), l10n{}) {}

    label() noexcept : label(tt::icon{}, l10n{}) {}

    label(label const &other) noexcept :
        _icon(other._icon), _fmt(other._fmt), _args(other._args->make_unique_copy())
    {
    }

    label &operator=(label const &other) noexcept
    {
        // Self-assignment is allowed.
        _icon = other._icon;
        _fmt = other._fmt;
        _args = other._args->make_unique_copy();
        return *this;
    }

    label(label &&other) noexcept = default;
    label &operator=(label &&other) noexcept = default;

    [[nodiscard]] bool has_icon() const noexcept
    {
        return static_cast<bool>(_icon);
    }

    [[nodiscard]] icon const &icon() const noexcept
    {
        return _icon;
    }

    void set_icon(tt::icon icon) noexcept
    {
        _icon = std::move(icon);
    }

    [[nodiscard]] bool has_text() const noexcept
    {
        return _fmt;
    }

    [[nodiscard]] std::string text() const noexcept
    {
        auto fmt_s = _fmt.get_translation();
        tt_axiom(_args);
        return _args->format(fmt_s);
    }

    template<typename... Args>
    void set_text(l10n fmt, Args &&... args) noexcept
    {
        _fmt = fmt;
        _args = std::make_unique<detail::label_arguments<std::remove_cvref_t<Args>...>>(std::forward<Args>(args)...);
    }

    [[nodiscard]] friend bool operator==(label const &lhs, label const &rhs) noexcept
    {
        return lhs._icon == rhs._icon && lhs._fmt == rhs._fmt && lhs._args->eq(*rhs._args);
    }

    [[nodiscard]] friend std::string to_string(label const &rhs) noexcept
    {
        return rhs.text();
    }

    friend std::ostream &operator<<(std::ostream &lhs, label const &rhs)
    {
        return lhs << to_string(rhs);
    }

private:
    tt::icon _icon;
    l10n _fmt;
    std::unique_ptr<detail::label_arguments_base> _args;
};

} // namespace tt