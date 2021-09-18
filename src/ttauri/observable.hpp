// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "type_traits.hpp"
#include "unfair_mutex.hpp"
#include "notifier.hpp"
#include <memory>
#include <vector>
#include <atomic>
#include <concepts>

namespace tt {
template<typename T>
class observable;

namespace detail {

template<typename T>
struct observable_impl;

template<typename T, bool Constant>
struct observable_proxy {
    using value_type = T;
    static constexpr bool is_atomic = may_be_atomic_v<T>;
    static constexpr bool is_constant = Constant;
    static constexpr bool is_locking = not is_atomic;
    static constexpr bool is_variable = not is_constant;

    ~observable_proxy()
    {
        if (actual) {
            if constexpr (is_locking or is_variable) {
                actual->unlock();
            }
            if constexpr (is_variable) {
                actual->notify_owners();
            }
        }
    }

    observable_proxy(observable_impl<T> &actual) : actual(&actual)
    {
        if constexpr (is_locking or is_variable) {
            this->actual->lock();
        }
    }

    observable_proxy(observable_proxy &&other) noexcept : actual(std::exchange(other.actual, nullptr)) {}

    observable_proxy &operator=(observable_proxy &&other) noexcept
    {
        actual = std::exchange(other.actual, nullptr);
        return *this;
    }

    observable_proxy(observable_proxy const &) = delete;
    observable_proxy &operator=(observable_proxy const &) = delete;

    // MSVC Compiler bug returning this with auto argument
    template<std::convertible_to<value_type> Value>
    observable_proxy &operator=(Value &&value) noexcept
    {
        tt_axiom(actual);
        actual->value = std::forward<decltype(value)>(value);
        return *this;
    }

    value_type operator*() const requires(is_atomic)
    {
        tt_axiom(actual);
        return actual->value.load();
    }

    value_type const &operator*() const requires(is_locking)
    {
        tt_axiom(actual);
        return actual->value;
    }

    value_type &operator*() requires(is_locking and is_variable)
    {
        tt_axiom(actual);
        return actual->value;
    }

    value_type const *operator->() const requires(is_locking)
    {
        tt_axiom(actual);
        return &(actual->value);
    }

    value_type *operator->() requires(is_locking and is_variable)
    {
        tt_axiom(actual);
        return &(actual->value);
    }

    explicit operator bool() const noexcept
    {
        tt_axiom(actual);
        return static_cast<bool>(actual->value);
    }

    template<typename... Args>
    auto operator()(Args &&...args) noexcept
    {
        tt_axiom(actual);
        return actual->value(std::forward<Args>(args)...);
    }

#define X(op) \
    [[nodiscard]] friend auto operator op(observable_proxy const &lhs, observable_proxy const &rhs) noexcept \
    { \
        tt_axiom(lhs.actual and rhs.actual); \
        return lhs.actual->value op rhs.actual->value; \
    } \
\
    [[nodiscard]] friend auto operator op(observable_proxy const &lhs, auto const &rhs) noexcept \
    { \
        tt_axiom(lhs.actual); \
        return lhs.actual->value op rhs; \
    } \
\
    [[nodiscard]] friend auto operator op(auto const &lhs, observable_proxy const &rhs) noexcept \
    { \
        tt_axiom(rhs.actual); \
        return lhs op rhs.actual->value; \
    }

    X(==)
    X(-)
#undef X

#define X(op) \
    [[nodiscard]] auto operator op() const noexcept \
    { \
        tt_axiom(actual); \
        return op actual->value; \
    }

    X(-)
#undef X

    // MSVC Internal compiler error
    template<typename Rhs>
    value_type operator+=(Rhs const &rhs) noexcept
    {
        tt_axiom(actual);
        return actual->value += rhs;
    }

    observable_impl<T> *actual = nullptr;
};

template<typename T>
struct observable_impl {
    static constexpr bool is_atomic = may_be_atomic_v<T>;

    using value_type = T;
    using atomic_value_type = std::conditional_t<is_atomic, std::atomic<value_type>, value_type>;
    using owner_type = observable<value_type>;

    atomic_value_type value;
    std::vector<owner_type *> owners;
    mutable unfair_mutex mutex;

    ~observable_impl()
    {
        tt_axiom(owners.empty());
    }

    observable_impl(observable_impl const &) = delete;
    observable_impl(observable_impl &&) = delete;
    observable_impl &operator=(observable_impl const &) = delete;
    observable_impl &operator=(observable_impl &&) = delete;

    observable_impl() noexcept : value() {}
    observable_impl(value_type const &value) noexcept : value(value) {}
    observable_impl(value_type &&value) noexcept : value(std::move(value)) {}

    observable_proxy<value_type, false> get() noexcept
    {
        return observable_proxy<value_type, false>(*this);
    }

    observable_proxy<value_type, true> cget() noexcept
    {
        return observable_proxy<value_type, true>(*this);
    }

    void add_owner(owner_type *owner) noexcept
    {
        ttlet lock = std::scoped_lock(mutex);
        owners.push_back(owner);
    }

    void remove_owner(owner_type *owner) noexcept
    {
        ttlet lock = std::scoped_lock(mutex);
        std::erase(owners, owner);
    }

    void lock() const noexcept
    {
        mutex.lock();
    }

    void unlock() const noexcept
    {
        mutex.unlock();
    }

    void notify_owners() const noexcept
    {
        mutex.lock();
        ttlet owners_copy = owners;
        mutex.unlock();

        for (ttlet owner : owners_copy) {
            owner->notify();
        }
    }

    void reseat_owners(std::shared_ptr<observable_impl> const &new_impl) noexcept
    {
        mutex.lock();

        tt_axiom(not owners.empty());
        auto keep_this_alive = owners.front()->pimpl;

        for (auto owner : owners) {
            owner->pimpl = new_impl;
            new_impl->add_owner(owner);
        }

        auto old_owners = std::move(owners);
        owners.clear();

        mutex.unlock();

        for (auto owner : old_owners) {
            owner->notify();
        }
    }
};

} // namespace detail

template<typename T>
class observable {
public:
    using value_type = T;
    using reference = detail::observable_proxy<value_type,false>;
    using const_reference = detail::observable_proxy<value_type,true>;
    using impl_type = detail::observable_impl<value_type>;
    using notifier_type = tt::notifier<void()>;
    using callback_ptr_type = notifier_type::callback_ptr_type;
    static constexpr bool is_atomic = impl_type::is_atomic;

    ~observable()
    {
        pimpl->remove_owner(this);
    }

    observable() noexcept : pimpl(std::make_shared<impl_type>())
    {
        pimpl->add_owner(this);
    }

    observable(observable const &other) noexcept : pimpl(other.pimpl)
    {
        pimpl->add_owner(this);
    }

    observable &operator=(observable const &other) noexcept
    {
        tt_return_on_self_assignment(other);

        pimpl->reseat_owners(other.pimpl);
        return *this;
    }

    const_reference cget() const noexcept
    {
        return pimpl->cget();
    }

    const_reference get() const noexcept
    {
        return pimpl->cget();
    }

    reference get() noexcept
    {
        return pimpl->get();
    }

    observable(std::convertible_to<value_type> auto &&value) noexcept :
        pimpl(std::make_shared<impl_type>(std::forward<decltype(value)>(value)))
    {
        pimpl->add_owner(this);
    }

    // MSVC Compiler bug returning this with auto argument
    template<std::convertible_to<value_type> Value>
    observable &operator=(Value &&value) noexcept
    {
        tt_axiom(pimpl);
        get() = std::forward<decltype(value)>(value);
        return *this;
    }

    value_type operator*() const noexcept
    {
        return *(cget());
    }

    detail::observable_proxy<value_type, true> operator->() const noexcept
    {
        return cget();
    }

    explicit operator bool() const noexcept
    {
        return static_cast<bool>(cget());
    }

#define X(op) \
    [[nodiscard]] friend auto operator op(observable const &lhs, observable const &rhs) noexcept \
    { \
        return lhs.cget() op rhs.cget(); \
    } \
\
    [[nodiscard]] friend auto operator op(observable const &lhs, auto const &rhs) noexcept \
    { \
        return lhs.cget() op rhs; \
    } \
\
    [[nodiscard]] friend auto operator op(auto const &lhs, observable const &rhs) noexcept \
    { \
        return lhs op rhs.cget(); \
    }

    X(==)
    X(-)
#undef X

#define X(op) \
    [[nodiscard]] auto operator op() const noexcept \
    { \
        return op cget(); \
    }

    X(-)

#undef X

    // MSVC Internal compiler error
    template<typename Rhs>
    value_type operator+=(Rhs const &rhs) noexcept
    {
        return get() += rhs;
    }

    callback_ptr_type subscribe(callback_ptr_type const &callback_ptr) noexcept
    {
        return notifier.subscribe(callback_ptr);
    }

    template<typename Callback>
    [[nodiscard]] callback_ptr_type subscribe(Callback &&callback) noexcept requires(std::is_invocable_v<Callback>)
    {
        auto callback_ptr = notifier.subscribe(std::forward<Callback>(callback));
        (*callback_ptr)();
        return callback_ptr;
    }

    void unsubscribe(callback_ptr_type const &callback_ptr) noexcept
    {
        notifier.unsubscribe(callback_ptr);
    }

private:
    std::shared_ptr<impl_type> pimpl;
    notifier_type notifier;

    void notify() const noexcept
    {
        notifier();
    }

    friend impl_type;
};

/** The value_type of an observable from the constructor argument type.
 * The argument type of an observable is the value_type or an observable<value_type>.
 * This is mostly used for creating CTAD guides.
 */
template<typename T>
struct observable_argument {
    using type = T;
};

template<typename T>
struct observable_argument<observable<T>> {
    using type = typename observable<T>::value_type;
};

template<typename T>
using observable_argument_t = typename observable_argument<T>::type;

} // namespace tt