// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unfair_mutex.hpp"
#include "concepts.hpp"
#include "notifier.hpp"
#include <memory>
#include <vector>
#include <atomic>
#include <concepts>
#include <type_traits>
#include <mutex>

namespace tt::inline v1 {
template<typename T>
class observable;

namespace detail {

/** The shared value, shared between observers.
 */
template<typename T>
struct observable_impl {
    using value_type = T;
    using owner_type = observable<value_type>;

    struct proxy_type {
        constexpr proxy_type(proxy_type const &) noexcept = delete;
        constexpr proxy_type &operator=(proxy_type const &) noexcept = delete;

        constexpr proxy_type(proxy_type &&other) noexcept :
            actual(std::exchange(other.actual, nullptr)),
            old_value(std::exchange(other.old_value, {}))
        {
        }

        constexpr proxy_type &operator=(proxy_type &&other) noexcept
        {
            if (actual) {
#if TT_BUILD_TYPE == TT_BT_DEBUG
                // This proxy will not be used anymore, so notifier_owners may cause new proxies to be openened.
                actual->rw_count = false;
#endif
                if (old_value != actual->value) {
                    actual->notifier_owners();
                }

            }

            actual = std::exchange(other.actual, nullptr);
            old_value = std::exchange(other.old_value, {});
        }

        constexpr proxy_type() noexcept : acctual(nullptr), old_value() {}

        constexpr proxy_type(observable_impl *actual) noexcept : actual(actual), old_value(actual->value)
        {
#if TT_BUILD_TYPE == TT_BT_DEBUG
            // Cannot open a read-write proxy when something already has a read proxy open.
            tt_assert(actual->ro_count == 0);
            // There may only be one read-write proxy.
            tt_assert(not std::exchange(actual->rw_count, true));
#endif
        }


        constexpr ~proxy_type()
        {
            if (actual) {
#if TT_BUILD_TYPE == TT_BT_DEBUG
                // This proxy will not be used anymore, so notifier_owners may cause new proxies to be openened.
                actual->rw_count = false;
#endif
                if (old_value != actual->value) {
                    actual->notifier_owners();
                }
            }
        }

        constexpr value_type *operator->() const noexcept
        {
            tt_axiom(actual);
            return &(actual->value);
        }

        constexpr value_type &operator*() const noexcept
        {
            tt_axiom(actual);
            return actual->value;
        }

        observable_impl *actual;
        value_type old_value;
    };

    struct const_proxy_type {
        constexpr const_proxy_type(const_proxy_type const &other) noexcept : actual(other.actual)
        {
#if TT_BUILD_TYPE == TT_BT_DEBUG
            if (actual) {
                tt_assert(actual->rw_count == false);
                tt_assert(actual->ro_count != 0);
                ++actual->ro_count;
            }
#endif
        }

        constexpr const_proxy_type(const_proxy_type &&other) noexcept : actual(std::exchange(other.actual, nullptr))
        {
#if TT_BUILD_TYPE == TT_BT_DEBUG
            if (actual) {
                tt_assert(actual->rw_count == false);
                tt_assert(actual->ro_count != 0);
            }
#endif
        }

        constexpr const_proxy_type &operator=(const_proxy_type const &other) noexcept 
        {
#if TT_BUILD_TYPE == TT_BT_DEBUG
            if (actual) {
                tt_assert(actual->rw_count == false);
                tt_assert(actual->ro_count != 0);
                --actual->ro_count;
            }
#endif
            actual = other.actual;
#if TT_BUILD_TYPE == TT_BT_DEBUG
            if (actual) {
                tt_assert(actual->rw_count == false);
                tt_assert(actual->ro_count != 0);
                ++actual->ro_count;
            }
#endif
        }

        constexpr const_proxy_type &operator=(const_proxy_type &&other) noexcept
        {
#if TT_BUILD_TYPE == TT_BT_DEBUG
            if (actual) {
                tt_assert(actual->rw_count == false);
                tt_assert(actual->ro_count != 0);
                --actual->ro_count;
            }
#endif
            actual = std::exchange(other.actual, nullptr);
        }

        constexpr const_proxy_type(proxy_type &&other) noexcept : actual(std::exchange(other.actual, nullptr))
        {
#if TT_BUILD_TYPE == TT_BT_DEBUG
            if (actual) {
                tt_assert(actual->rw_count == true);
                tt_assert(actual->ro_count == 0);
                actual->rw_count = false;
                ++actual->ro_count;
            }
#endif
        }

        constexpr const_proxy_type &operator=(proxy_type &&other) noexcept
        {
#if TT_BUILD_TYPE == TT_BT_DEBUG
            if (actual) {
                tt_assert(actual->rw_count == false);
                tt_assert(actual->ro_count != 0);
                --actual->ro_count;
            }
#endif
            actual = std::exchange(other.actual, nullptr);
#if TT_BUILD_TYPE == TT_BT_DEBUG
            if (actual) {
                tt_assert(actual->rw_count == true);
                tt_assert(actual->ro_count == 0);
                actual->rw_count = false;
                ++actual->ro_count;
            }
#endif
        }

        constexpr const_proxy_type() noexcept : actual(nullptr)  {}

        constexpr const_proxy_type(observable_impl *actual) noexcept : actual(actual)
        {
#if TT_BUILD_TYPE == TT_BT_DEBUG
            if (actual) {
                // Cannot open a read-only proxy with a read-write proxy.
                tt_assert(actual->rw_count == false);
                ++actual->ro_count;
             }
#endif
        }

        constexpr ~const_proxy_type()
        {
#if TT_BUILD_TYPE == TT_BT_DEBUG
            if (actual) {
                tt_assert(actual->rw_count == false);
                tt_assert(actual->ro_count != 0);
                --actual->ro_count;
            }
#endif
        }

        constexpr value_type const *operator->() const noexcept
        {
            tt_axiom(actual);
            return &(actual->value);
        }

        constexpr value_type const &operator*() const noexcept
        {
            tt_axiom(actual);
            return actual->value;
        }

        observable_impl *actual;
    };

    /** Mutex used to handle ownership of observable_impl.
     */
    inline static unfair_mutex mutex;

    value_type value;

    std::vector<owner_type *> owners;

    ~observable_impl()
    {
        tt_axiom(owners.empty());
    }

    observable_impl(observable_impl const &) = delete;
    observable_impl(observable_impl &&) = delete;
    observable_impl &operator=(observable_impl const &) = delete;
    observable_impl &operator=(observable_impl &&) = delete;

    observable_impl() noexcept : value() {}
    observable_impl(std::convertible_to<value_type> auto &&value) noexcept : value(tt_forward(value)) {}

    observable_proxy<value_type, false> get() noexcept
    {
        return observable_proxy<value_type, false>(*this);
    }

    observable_proxy<value_type, true> cget() noexcept
    {
        return observable_proxy<value_type, true>(*this);
    }

    void notify_owners() const noexcept
    {
        ttlet lock = std::scoped_lock(mutex);

        for (ttlet &owner: owners) {
            owner->_notifier(value);
        }
    }

    /** Add an observer as one of the owners of the shared-value.
     *
     * @param owner A reference to observer
     */
    void add_owner(owner_type &owner) noexcept
    {
        tt_axiom(mutex.is_locked());
        tt_axiom(std::find(owners.cbegin(), owners.cend(), &owner) == owners.cend());

        owners.push_back(&owner);
    }

    /** Remove an observer as one of the owners of the shared-value.
     *
     * @param owner A reference to observer
     */
    void remove_owner(owner_type &owner) noexcept
    {
        tt_axiom(mutex.is_locked());
        ttlet nr_erased = std::erase(owners, &owner);
        tt_axiom(nr_erased == 1);
    }

    void reseat_owners(std::shared_ptr<observable_impl> const &new_impl) noexcept
    {
        tt_axiom(mutex.is_locked());
        tt_axiom(not owners.empty());

        auto keep_this_alive = owners.front()->_pimpl;

        for (auto owner : owners) {
            owner->_pimpl = new_impl;
            new_impl->owners.push_back(owner);
            owner->_notifier(value);
        }
        owners.clear();
    }
};

} // namespace detail

/** A value which can be observed for modifications.
 *
 * An observable is used to share a value between different objects, and
 * for those objects to be notified when this shared-value is modified.
 *
 * Typically objects will own an instance of an observable and `subscribe()`
 * one of its methods to the observable. By assigning the observables of each
 * object to each other they will share the same value.
 * Now if one object changes the shared value, the other objects will get notified.
 *
 * When assigning observables to each other, the tokens
 * to the observable remain unmodified. However which value is shared is shown in the
 * example below:
 *
 * ```
 * auto a = observable<int>{1};
 * auto b = observable<int>{5};
 * auto c = observable<int>{42};
 * auto d = observable<int>{9};
 *
 * a = b; // both 'a' and 'b' share the value 5.
 * b = c; // 'a', 'b' and 'c' all share the value 42.
 * b = d; // 'a', 'b', 'c' and 'd' all share the value 9.
 * ```
 *
 * A proxy object is returned when dereferencing an observable. The
 * callbacks are called when both the value has changed and the
 * lifetime of all non-scalar proxy objects in the system has ended.
 *
 * A proxy of a non-scalar observable holds a mutex. It may be useful
 * to extend the lifetime of a proxy to handle multiple steps atomically.
 * However due to the mutex held, it may be possible to dead-lock when
 * the lifetime of multiple proxy objects are extended in different orders.
 *
 * Constant proxies are more efficient than non-constant proxies. You can
 * get a non-constant proxy using the `cget()` function. Many of the
 * operations available directly on the observable uses constant proxies
 * internally for this reason.
 *
 * @tparam T The type of the value to be observed.
 */
template<typename T>
class observable {
public:
    using value_type = T;
    using reference = detail::observable_proxy<value_type, false>;
    using const_reference = detail::observable_proxy<value_type, true>;
    using impl_type = detail::observable_impl<value_type>;
    using notifier_type = notifier<void(value_type)>;
    using token_type = notifier_type::token_type;
    static constexpr bool is_atomic = std::is_scalar_v<value_type>;

    ~observable()
    {
        ttlet lock = std::scoped_lock(impl_type::mutex);
        _pimpl->remove_owner(*this);
    }

    /** Construct an observable.
     *
     * The observer is created with a value that is default constructed.
     */
    observable() noexcept : _pimpl(std::make_shared<impl_type>())
    {
        ttlet lock = std::scoped_lock(impl_type::mutex);
        _pimpl->add_owner(*this);
    }

    /** Construct an observable and chain it to another.
     *
     * The new observable will share a value with the other observable.
     *
     * @param other The other observable to share the value with.
     */
    observable(observable const &other) noexcept : _pimpl(other._pimpl)
    {
        ttlet lock = std::scoped_lock(impl_type::mutex);
        _pimpl->add_owner(*this);
    }

    /** Chain with another observable.
     *
     * Replace the current shared value, with the value of the other observer.
     * This means the other observers that share the current value will also
     * be reseated with the new value.
     *
     * @post observers sharing the same value will be modified to share the new value.
     * @post subscribers are notified
     * @param other The other observable to share the value with.
     */
    observable &operator=(observable const &other) noexcept
    {
        if (this == &other or _pimpl == other._pimpl) {
            return *this;
        }

        tt_axiom(_pimpl);
        ttlet lock = std::scoped_lock(impl_type::mutex);
        _pimpl->reseat_owners(other._pimpl);
        return *this;
    }

    /** Get a constant reference to the shared value.
     *
     * In reality the reference is a proxy object which makes available
     * operations to be used with the shared value. This proxy object will
     * make sure any state is done atomically.
     *
     * @return A reference to the shared value.
     */
    const_reference cget() const noexcept
    {
        tt_axiom(_pimpl);
        return _pimpl->cget();
    }

    /** Get a constant reference to the shared value.
     *
     * In reality the reference is a proxy object which makes available
     * operations to be used with the shared value. This proxy object will
     * make sure any state is done atomically.
     *
     * @return A reference to the shared value.
     */
    const_reference get() const noexcept
    {
        tt_axiom(_pimpl);
        return _pimpl->cget();
    }

    /** Get a writable reference to the shared-value.
     *
     * In reality the reference is a proxy object which makes available
     * operations to be used with the shared-value. This proxy object will
     * make sure any state is done atomically and that subscribers
     * are notified when the proxy's lifetime has ended.
     *
     * @post subscribers are notified after reference's lifetime has ended.
     * @return A reference to the shared value.
     */
    reference get() noexcept
    {
        tt_axiom(_pimpl);
        return _pimpl->get();
    }

    /** Construct an observable with its value set.
     *
     * @param value The value to assign to the shared-value.
     */
    observable(std::convertible_to<value_type> auto &&value) noexcept :
        _pimpl(std::make_shared<impl_type>(tt_forward(value)))
    {
        ttlet lock = std::scoped_lock(impl_type::mutex);
        _pimpl->add_owner(*this);
    }

    /** Assign a new value.
     *
     * @post subscribers are notified.
     * @param value The value to assign to the shared-value.
     */
    observable &operator=(std::convertible_to<value_type> auto &&value) noexcept
    {
        tt_axiom(_pimpl);
        get() = tt_forward(value);
        return *this;
    }

    token_type subscribe(std::invocable<value_type> auto &&callback) noexcept
    {
        return _notifier.subscribe(tt_forward(callback));
    }

    /** Get copy of the shared-value.
     *
     * @return a copy of the shared-value.
     */
    value_type operator*() const noexcept requires(is_atomic)
    {
        return *(cget());
    }

    detail::observable_proxy<value_type, true> operator*() const noexcept requires(not is_atomic)
    {
        return cget();
    }

    detail::observable_proxy<value_type, false> operator*() noexcept requires(not is_atomic)
    {
        return get();
    }

    detail::observable_proxy<value_type, true> operator->() const noexcept
    {
        return cget();
    }

    detail::observable_proxy<value_type, false> operator->() noexcept
    {
        return get();
    }

    explicit operator bool() const noexcept
    {
        return static_cast<bool>(*cget());
    }

    auto operator co_await() const noexcept
    {
        return _notifier.operator co_await();
    }

    auto operator++() noexcept
    {
        return ++(get());
    }

    auto operator--() noexcept
    {
        return --(get());
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
    X(<=>)
    X(+)
    X(-)
#undef X

#define X(op) \
    [[nodiscard]] auto operator op() const noexcept \
    { \
        return op cget(); \
    }

    X(-)

#undef X

#define X(op) \
    template<typename Rhs> \
    value_type operator op(Rhs const &rhs) noexcept \
    { \
        return get() op rhs; \
    }

    X(+=)
    X(-=)
#undef X

private : std::shared_ptr<impl_type> _pimpl;
    notifier_type _notifier;
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

} // namespace tt::inline v1
