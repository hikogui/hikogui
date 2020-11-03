// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "strings.hpp"
#include "observable.hpp"
#include "icon.hpp"
#include "text/translation.hpp"
#include "stencils/text_stencil.hpp"
#include <string>
#include <type_traits>
#include <memory>

namespace tt {
namespace detail {

class l10n_label_arguments_base {
public:
    l10n_label_arguments_base() noexcept = default;
    virtual ~l10n_label_arguments_base() = default;
    l10n_label_arguments_base(l10n_label_arguments_base const &) = delete;
    l10n_label_arguments_base(l10n_label_arguments_base &&) = delete;
    l10n_label_arguments_base &operator=(l10n_label_arguments_base const &) = delete;
    l10n_label_arguments_base &operator=(l10n_label_arguments_base &&) = delete;

    [[nodiscard]] virtual std::u8string format(std::u8string_view fmt) const noexcept = 0;
    [[nodiscard]] virtual std::unique_ptr<l10n_label_arguments_base> make_unique_copy() const noexcept = 0;
    [[nodiscard]] virtual bool eq(l10n_label_arguments_base &other) const noexcept = 0;
};

template<typename... Args>
class l10n_label_arguments : public l10n_label_arguments_base {
public:
    l10n_label_arguments(Args &&... args) noexcept : _args(std::forward<Args>(args)...) {}

    [[nodiscard]] std::u8string format(std::u8string_view fmt) const noexcept override
    {
        return std::apply(
            [&fmt](auto const &... args) {
                return tt::format(fmt, args...);
            },
            _args);
    }

    [[nodiscard]] std::unique_ptr<l10n_label_arguments_base> make_unique_copy() const noexcept override
    {
        return std::apply(
            [](auto const &... args) {
                return std::make_unique<l10n_label_arguments<Args...>>(args...);
            },
            _args);
    }

    [[nodiscard]] bool eq(l10n_label_arguments_base &other) const noexcept override
    {
        auto *other_ = dynamic_cast<l10n_label_arguments *>(&other);
        if (other_ == nullptr) {
            return false;
        }

        return _args == other_->_args;
    }


private:
    std::tuple<std::remove_cvref_t<Args>...> _args;
};

} // namespace detail

/** A localizable string.
 * Used by gettext to extract all msgids from the program into the .pot file.
 */
class l10n {
public:
    l10n() noexcept : msgid() {}
    l10n(l10n const &) noexcept = default;
    l10n(l10n &&) noexcept = default;
    l10n &operator=(l10n const &) noexcept = default;
    l10n &operator=(l10n &&) noexcept = default;

    l10n(std::u8string_view msgid) noexcept : msgid(msgid) {}
    l10n(std::string_view msgid) noexcept
    {
        this->msgid = std::u8string(reinterpret_cast<char8_t const *>(msgid.data()), msgid.size());
    }

private:
    std::u8string msgid;

    friend class l10n_label;
};

/** A localized text + icon label.
 */
class l10n_label {
public:
    template<typename... Args>
    l10n_label(tt::icon icon, l10n fmt, Args &&... args) noexcept :
        _icon(std::move(icon)),
        _msgid(std::move(fmt.msgid)),
        _args(std::make_unique<detail::l10n_label_arguments<Args...>>(std::forward<Args>(args)...))
    {
    }

    template<typename... Args>
    l10n_label(l10n fmt, Args &&... args) noexcept : l10n_label(tt::icon{}, std::move(fmt), std::forward<Args>(args)...)
    {
    }

    l10n_label(tt::icon icon) noexcept : l10n_label(std::move(icon), l10n{})
    {
    }

    l10n_label() noexcept : l10n_label(tt::icon{}, std::u8string_view{}) {}

    l10n_label(l10n_label const &other) noexcept :
        _icon(other._icon), _msgid(other._msgid), _args(other._args->make_unique_copy())
    {
    }

    l10n_label &operator=(l10n_label const &other) noexcept
    {
        _icon = other._icon;
        _msgid = other._msgid;
        _args = other._args->make_unique_copy();
        return *this;
    }

    l10n_label(l10n_label &&other) noexcept = default;
    l10n_label &operator=(l10n_label &&other) noexcept = default;

    [[nodiscard]] operator std::u8string() const noexcept
    {
        auto fmt = get_translation(_msgid);
        tt_assume(_args);
        return _args->format(fmt);
    }

    [[nodiscard]] icon icon() const noexcept
    {
        return _icon;
    }

    [[nodiscard]] std::unique_ptr<stencil> make_stencil(Alignment alignment) const noexcept
    {
        return _icon.make_stencil(alignment);
    }

    [[nodiscard]] std::unique_ptr<stencil> make_stencil(Alignment alignment, TextStyle style) const noexcept
    {
        return std::make_unique<text_stencil>(alignment, to_u8string(*this), style);
    }

    [[nodiscard]] friend bool operator==(l10n_label const &lhs, l10n_label const &rhs) noexcept
    {
        return lhs._icon == rhs._icon && lhs._msgid == rhs._msgid &&
            lhs._args->eq(*rhs._args);
    }

    [[nodiscard]] friend std::u8string to_u8string(l10n_label const &rhs) noexcept {
        return static_cast<std::u8string>(rhs);
    }

    [[nodiscard]] friend std::string to_string(l10n_label const &rhs) noexcept
    {
        auto tmp = to_u8string(rhs);
        return std::string{reinterpret_cast<char const *>(tmp.data()), tmp.size()};
    }

    friend std::ostream &operator<<(std::ostream &lhs, l10n_label const &rhs)
    {
        return lhs << to_string(rhs);
    }

private:
    tt::icon _icon;
    std::u8string _msgid;
    std::unique_ptr<detail::l10n_label_arguments_base> _args;
};


namespace detail {

template<>
class observable_value<l10n_label> final : public observable_base<l10n_label> {
public:
    using super = observable_base<l10n_label>;

    observable_value() noexcept : super(), _value() {
        _language_list_callback = language::preferred_languages.subscribe([this](auto...) {
            notify({}, load());
        });
    }

    observable_value(l10n_label const &value) noexcept : super(), _value(value) {
        _language_list_callback = language::preferred_languages.subscribe([this](auto...) {
            notify({}, load());
        });
    }

    l10n_label load() const noexcept override
    {
        ttlet lock = std::scoped_lock(this->_mutex);
        return _value;
    }

    bool store(l10n_label const &new_value) noexcept override
    {
        this->_mutex.lock();

        ttlet old_value = _value;
        if constexpr (std::equality_comparable<l10n_label>) {
            if (new_value != old_value) {
                _value = new_value;
                this->_mutex.unlock();
                this->notify(old_value, new_value);
                return true;
            } else {
                this->_mutex.unlock();
                return false;
            }

        } else {
            _value = new_value;
            this->_mutex.unlock();
            this->notify(old_value, new_value);
            return true;
        }
    }

private:
    l10n_label _value;
    typename decltype(language::preferred_languages)::callback_ptr_type _language_list_callback;
};

}

} // namespace tt