// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "assert.hpp"
#include "misc.hpp"
#include "type_traits.hpp"
#include "concepts.hpp"
#include "terminate.hpp"
#include "../macros.hpp"
#include <type_traits>
#include <memory>
#include <concepts>

hi_export_module(utility.not_null);

hi_export namespace hi { inline namespace v1 {

template<nullable_pointer T>
class not_null {
public:
    using element_type = std::pointer_traits<T>::element_type;
    using pointer = element_type *;
    using reference = element_type &;

    ~not_null() = default;
    not_null() = delete;
    not_null(nullptr_t) = delete;
    not_null &operator=(nullptr_t) = delete;

    not_null(intrinsic_t, T &&o) noexcept : _p(std::move(o)) {}

    template<nullable_pointer O>
    not_null(not_null<O> const &o) noexcept
        requires requires { this->_p = o._p; }
     : _p(o._p)
    {
    }

    template<nullable_pointer O>
    not_null &operator=(not_null<O> const &o) noexcept
        requires requires { this->_p = o._p; }
    {
        if constexpr (std::same_as<std::remove_cvref_t<decltype(*this)>, std::remove_cvref_t<decltype(o)>>) {
            if (this == std::addressof(o)) {
                return *this;
            } 
        }

        _p = o._p;
        return *this;
    }

    template<nullable_pointer O>
    not_null(not_null<O> &&o) noexcept
        requires requires { this->_p = std::move(o._p); }
     : _p(std::move(o._p))
    {
    }

    template<nullable_pointer O>
    not_null &operator=(not_null<O> &&o) noexcept
        requires requires { this->_p = std::move(o._p); }
    {
         if constexpr (std::same_as<std::remove_cvref_t<decltype(*this)>, std::remove_cvref_t<decltype(o)>>) {
            if (this == std::addressof(o)) {
                return *this;
            } 
        }
        
        _p = std::move(o._p);
        return *this;
    }

    template<nullable_pointer O>
    not_null(O &&o) noexcept
        requires requires { this->_p = std::forward<O>(o); } :
        _p(std::forward<O>(o))
    {
        hi_assert_not_null(_p);
    }

    template<nullable_pointer O>
    not_null &operator=(O &&o) noexcept
        requires requires { this->_p = std::forward<O>(o); }
    {
        hi_assert_not_null(o);
        _p = std::forward<O>(o);
        return *this;
    }

    // XXX: due to a bug in the standard you can not create a reference
    //      qualifier overload set (ambiguity). To be able to properly use
    //      not_null<std::unique_ptr<T>> we will need return a rvalue reference
    //      when this is a rvalue reference, this can only be done using
    //      C++23 deducing this.

    operator T() const noexcept
    {
        return _p;
    }

    [[nodiscard]] pointer get() const noexcept
    {
        hilet r = [this]{
            if constexpr (std::is_pointer_v<T>) {
                return _p;
            } else {
                return _p.get();
            }
        }();

        hi_assume(r != nullptr);
        return r;
    }

    [[deprecated("not_null<> pointers are always true")]] explicit operator bool() const noexcept
    {
        return true;
    }

    [[deprecated("not_null<> pointers are never equal to nullptr")]] [[nodiscard]] bool operator==(nullptr_t) const noexcept
    {
        return false;
    }

    [[nodiscard]] reference operator*() const noexcept
    {
        return *_p;
    }

    [[nodiscard]] pointer operator->() const noexcept
    {
        return get();
    }

private:
    T _p;

    template<nullable_pointer T>
    friend class not_null;
};

template<typename T, typename... Args>
[[nodiscard]] not_null<std::unique_ptr<T>> make_unique_not_null(Args &&... args)
{
    return not_null<std::unique_ptr<T>>{intrinsic, std::make_unique<T>(std::forward<Args>(args)...)};
}

template<typename T, typename... Args>
[[nodiscard]] not_null<std::shared_ptr<T>> make_shared_not_null(Args &&... args)
{
    return not_null<std::shared_ptr<T>>{intrinsic, std::make_shared<T>(std::forward<Args>(args)...)};
}

template<typename T>
[[nodiscard]] not_null<T *> make_not_null(T *ptr) noexcept
{
    return not_null<T *>{ptr};
}

template<typename T>
[[nodiscard]] not_null<T *> make_not_null(T &ptr) noexcept
{
    return not_null<T *>{intrinsic, std::addressof(ptr)};
}

template<typename T>
[[nodiscard]] not_null<T *> make_not_null(T &&ptr) noexcept = delete;

}} // namespace hi::v1
