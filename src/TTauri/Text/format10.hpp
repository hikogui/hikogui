// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <fmt/format.h>
#include <utility>
#include <tuple>
#include <memory>
#include <string>
#include <locale>
#include "translation.hpp"

namespace tt {

class format10_base
{
protected:
    std::string fmt;

public:
    format10_base(std::string fmt) noexcept :
        fmt(std::move(fmt)) {}

    virtual ~format10_base() = default;

    virtual operator std::string () const noexcept = 0;

    virtual std::unique_ptr<format10_base> make_unique_copy() const noexcept = 0;

    [[nodiscard]] virtual bool equal_to9(format10_base &other) const noexcept = 0;
};

template<typename... Args>
class format10_impl : public format10_base
{
    std::tuple<Args...> args;

public:
    template<typename Fmt>
    format10_impl(Fmt &&fmt, Args &&... args) noexcept :
        format10_base(std::forward<Fmt>(fmt)),
        args(std::forward<Args>(args)...) {}

    operator std::string () const noexcept override {
        auto locale = std::locale{};
        auto translated_fmt = get_translation(fmt);

        if constexpr (sizeof...(Args) == 0) {
            return std::string{translated_fmt};
        } else {
            return std::apply(fmt::format, std::tuple_cat(std::tuple(locale, translated_fmt), args));
        }
    }

    std::unique_ptr<format10_base> make_unique_copy() const noexcept override {
        if constexpr (sizeof...(Args) == 0) {
            return std::make_unique<format10_impl>(fmt);

        } else {
            return std::apply(std::make_unique<format10_impl>, std::tuple_cat(std::tuple(fmt), args));
        }
    }

    [[nodiscard]] bool equal_to9(format10_base &other) const noexcept override {
        auto *other_ = dynamic_cast<format10_impl *>(&other);
        return other_ && this->fmt == other_->fmt && this->args == other_->args;
    }
};

class format10 {
    std::unique_ptr<format10_base> impl;

public:
    format10() noexcept :
        impl(std::make_unique<format10_impl<>>(""s)) {}

    template<typename Fmt, typename... Args>
    format10(Fmt &&fmt, Args &&... args) noexcept :
        impl(std::make_unique<format10_impl<Args...>>(std::forward<Fmt>(fmt), std::forward<Args>(args)...)) {}

    format10(format10 &&other) noexcept = default;
    format10 &operator=(format10 &&other) noexcept = default;

    format10(format10 const &other) noexcept :
        impl(other.impl->make_unique_copy()) {
        tt_assume(other.impl);
    }

    format10 &operator=(format10 const &other) noexcept {
        tt_assume(other.impl);
        impl = other.impl->make_unique_copy();
        return *this;
    }

    operator std::string () const noexcept {
        tt_assume(impl);
        return static_cast<std::string>(*impl);
    }

    [[nodiscard]] friend bool operator==(format10 const &lhs, format10 const &rhs) noexcept {
        tt_assume(lhs.impl);
        tt_assume(rhs.impl);
        return lhs.impl->equal_to9(*rhs.impl);
    }

    [[nodiscard]] friend bool operator!=(format10 const &lhs, format10 const &rhs) noexcept {
        return !(lhs == rhs);
    }

};


}

// Add specialization for observable_cast
#include "detail/observable_cast_format10.hpp"
