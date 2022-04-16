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

namespace hi::inline v1 {
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
        constexpr proxy_type(proxy_type const&) noexcept = delete;
        constexpr proxy_type& operator=(proxy_type const&) noexcept = delete;

        constexpr proxy_type(proxy_type&& other) noexcept :
            actual(std::exchange(other.actual, nullptr)), old_value(std::exchange(other.old_value, {}))
        {
        }

        constexpr proxy_type& operator=(proxy_type&& other) noexcept
        {
            if (actual) {
#if HI_BUILD_TYPE == HI_BT_DEBUG
                // This proxy will not be used anymore, so notifier_owners may cause new proxies to be opened.
                actual->rw_count = false;
#endif
                if (old_value != actual->value) {
                    actual->notifier_owners();
                }
            }

            actual = std::exchange(other.actual, nullptr);
            old_value = std::exchange(other.old_value, {});
        }

        constexpr proxy_type() noexcept : actual(nullptr), old_value() {}

        constexpr proxy_type(observable_impl *actual) noexcept : actual(actual), old_value(actual->value)
        {
#if HI_BUILD_TYPE == HI_BT_DEBUG
            // Cannot open a read-write proxy when something already has a read proxy open.
            hi_assert(actual->ro_count == 0);
            // There may only be one read-write proxy.
            hi_assert(not std::exchange(actual->rw_count, true));
#endif
        }

        constexpr ~proxy_type()
        {
            if (actual) {
#if HI_BUILD_TYPE == HI_BT_DEBUG
                // This proxy will not be used anymore, so notifier_owners may cause new proxies to be opened.
                actual->rw_count = false;
#endif
                if (old_value != actual->value) {
                    actual->notify_owners();
                }
            }
        }

        constexpr operator value_type &() const noexcept
        {
            hi_axiom(actual);
            return actual->value;
        }

        constexpr value_type *operator->() const noexcept
        {
            hi_axiom(actual);
            return std::addressof(actual->value);
        }

        constexpr value_type& operator*() const noexcept
        {
            hi_axiom(actual);
            return actual->value;
        }

        constexpr value_type *operator&() const noexcept
        {
            hi_axiom(actual);
            return std::addressof(actual->value);
        }

        observable_impl *actual;
        value_type old_value;
    };

    struct const_proxy_type {
        constexpr const_proxy_type(const_proxy_type const& other) noexcept : actual(other.actual)
        {
#if HI_BUILD_TYPE == HI_BT_DEBUG
            if (actual) {
                hi_assert(actual->rw_count == false);
                hi_assert(actual->ro_count != 0);
                ++actual->ro_count;
            }
#endif
        }

        constexpr const_proxy_type(const_proxy_type&& other) noexcept : actual(std::exchange(other.actual, nullptr))
        {
#if HI_BUILD_TYPE == HI_BT_DEBUG
            if (actual) {
                hi_assert(actual->rw_count == false);
                hi_assert(actual->ro_count != 0);
            }
#endif
        }

        constexpr const_proxy_type& operator=(const_proxy_type const& other) noexcept
        {
#if HI_BUILD_TYPE == HI_BT_DEBUG
            if (actual) {
                hi_assert(actual->rw_count == false);
                hi_assert(actual->ro_count != 0);
                --actual->ro_count;
            }
#endif
            actual = other.actual;
#if HI_BUILD_TYPE == HI_BT_DEBUG
            if (actual) {
                hi_assert(actual->rw_count == false);
                hi_assert(actual->ro_count != 0);
                ++actual->ro_count;
            }
#endif
        }

        constexpr const_proxy_type& operator=(const_proxy_type&& other) noexcept
        {
#if HI_BUILD_TYPE == HI_BT_DEBUG
            if (actual) {
                hi_assert(actual->rw_count == false);
                hi_assert(actual->ro_count != 0);
                --actual->ro_count;
            }
#endif
            actual = std::exchange(other.actual, nullptr);
        }

        constexpr const_proxy_type(proxy_type&& other) noexcept : actual(std::exchange(other.actual, nullptr))
        {
#if HI_BUILD_TYPE == HI_BT_DEBUG
            if (actual) {
                hi_assert(actual->rw_count == true);
                hi_assert(actual->ro_count == 0);
                actual->rw_count = false;
                ++actual->ro_count;
            }
#endif
        }

        constexpr const_proxy_type& operator=(proxy_type&& other) noexcept
        {
#if HI_BUILD_TYPE == HI_BT_DEBUG
            if (actual) {
                hi_assert(actual->rw_count == false);
                hi_assert(actual->ro_count != 0);
                --actual->ro_count;
            }
#endif
            actual = std::exchange(other.actual, nullptr);
#if HI_BUILD_TYPE == HI_BT_DEBUG
            if (actual) {
                hi_assert(actual->rw_count == true);
                hi_assert(actual->ro_count == 0);
                actual->rw_count = false;
                ++actual->ro_count;
            }
#endif
        }

        constexpr const_proxy_type() noexcept : actual(nullptr) {}

        constexpr const_proxy_type(observable_impl *actual) noexcept : actual(actual)
        {
#if HI_BUILD_TYPE == HI_BT_DEBUG
            if (actual) {
                // Cannot open a read-only proxy with a read-write proxy.
                hi_assert(actual->rw_count == false);
                ++actual->ro_count;
            }
#endif
        }

        constexpr ~const_proxy_type()
        {
#if HI_BUILD_TYPE == HI_BT_DEBUG
            if (actual) {
                hi_assert(actual->rw_count == false);
                hi_assert(actual->ro_count != 0);
                --actual->ro_count;
            }
#endif
        }

        constexpr operator value_type const &() const noexcept
        {
            hi_axiom(actual);
            return actual->value;
        }

        constexpr value_type const *operator->() const noexcept
        {
            hi_axiom(actual);
            return std::addressof(actual->value);
        }

        constexpr value_type const& operator*() const noexcept
        {
            hi_axiom(actual);
            return actual->value;
        }

        constexpr value_type const *operator&() const noexcept
        {
            hi_axiom(actual);
            return std::addressof(actual->value);
        }

        observable_impl *actual;
    };

    value_type value;
    std::vector<owner_type *> owners;
#if HI_BUILD_TYPE == HI_BT_DEBUG
    size_t ro_count = 0;
    bool rw_count = false;
#endif

    ~observable_impl()
    {
        hi_axiom(owners.empty());
    }

    observable_impl(observable_impl const&) = delete;
    observable_impl(observable_impl&&) = delete;
    observable_impl& operator=(observable_impl const&) = delete;
    observable_impl& operator=(observable_impl&&) = delete;

    observable_impl() noexcept : value() {}
    observable_impl(std::convertible_to<value_type> auto&& value) noexcept : value(hi_forward(value)) {}

    proxy_type proxy() noexcept
    {
        return this;
    }

    const_proxy_type const_proxy() noexcept
    {
        return this;
    }

    void notify_owners() const noexcept
    {
        for (hilet& owner : owners) {
            owner->_notifier.post(value);
        }
    }

    /** Add an observer as one of the owners of the shared-value.
     *
     * @param owner A reference to observer
     */
    void add_owner(owner_type& owner) noexcept
    {
        hi_axiom(std::find(owners.cbegin(), owners.cend(), &owner) == owners.cend());

        owners.push_back(&owner);
    }

    /** Remove an observer as one of the owners of the shared-value.
     *
     * @param owner A reference to observer
     */
    void remove_owner(owner_type& owner) noexcept
    {
        hilet nr_erased = std::erase(owners, &owner);
        hi_axiom(nr_erased == 1);
    }

    void reseat_owners(std::shared_ptr<observable_impl> const& new_impl) noexcept
    {
        hi_axiom(not owners.empty());

        auto keep_this_alive = owners.front()->_pimpl;

        for (auto owner : owners) {
            owner->_pimpl = new_impl;
            new_impl->owners.push_back(owner);
            owner->_notifier.post(value);
        }
        owners.clear();
    }
};

} // namespace detail

/** An observable value.
 *
 * Typically owners of an observable will subscribe a callback that will
 * be called when the observed value has changed.
 *
 * Multiple observables will be able to share the same value by assigning
 * observables to each other, as shown in the example below. The callback
 * subscribtion on each observable will not change with these assignments.
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
 * Access to the value is done through either a proxy or const-proxy.
 * Only one proxy xor multiple const-proxies can be outstanding at a time.
 * This is checked on debug builds.
 *
 * The const-proxy is cheap and possibly completely optimized away for accessing
 * the value read-only. The (non-const) proxy may be quite expensive as it makes
 * a copy of the value to compare against to determine if change-notification is needed.
 *
 * For this reason almost all operators of `observable` will work on const-proxies.
 *
 * If you want to modify the value, or make multiple modifications you may explicitly
 * use the `proxy()` function to get a proxy object and dereference it.
 *
 *
 * @tparam T The type of the value to be observed.
 */
template<typename T>
class observable {
public:
    using value_type = T;
    using impl_type = detail::observable_impl<value_type>;
    using proxy_type = impl_type::proxy_type;
    using const_proxy_type = impl_type::const_proxy_type;
    using reference = value_type&;
    using const_reference = value_type const&;
    using notifier_type = notifier<void(value_type)>;
    using token_type = notifier_type::token_type;
    using awaiter_type = notifier_type::awaiter_type;

    ~observable()
    {
        _pimpl->remove_owner(*this);
    }

    /** Construct an observable.
     *
     * The observer is created with a value that is default constructed.
     */
    observable() noexcept : _pimpl(std::make_shared<impl_type>())
    {
        _pimpl->add_owner(*this);
    }

    /** Construct an observable and chain it to another.
     *
     * The new observable will share a value with the other observable.
     *
     * @param other The other observable to share the value with.
     */
    observable(observable const& other) noexcept : _pimpl(other._pimpl)
    {
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
    observable& operator=(observable const& other) noexcept
    {
        if (this == &other or _pimpl == other._pimpl) {
            return *this;
        }

        hi_axiom(_pimpl);
        _pimpl->reseat_owners(other._pimpl);
        return *this;
    }

    token_type subscribe(std::invocable<value_type> auto&& callback) noexcept
    {
        return _notifier.subscribe(hi_forward(callback));
    }

    awaiter_type operator co_await() const noexcept
    {
        return _notifier.operator co_await();
    }

    /** proxy a constant reference to the shared value.
     *
     * In reality the reference is a proxy object which makes available
     * operations to be used with the shared value. This proxy object will
     * make sure any state is done atomically.
     *
     * @return A reference to the shared value.
     */
    const_proxy_type const_proxy() const noexcept
    {
        hi_axiom(_pimpl);
        return _pimpl->const_proxy();
    }

    /** proxy a constant reference to the shared value.
     *
     * In reality the reference is a proxy object which makes available
     * operations to be used with the shared value. This proxy object will
     * make sure any state is done atomically.
     *
     * @return A reference to the shared value.
     */
    proxy_type proxy() const noexcept
    {
        hi_axiom(_pimpl);
        return _pimpl->const_proxy();
    }

    /** proxy a writable reference to the shared-value.
     *
     * In reality the reference is a proxy object which makes available
     * operations to be used with the shared-value. This proxy object will
     * make sure any state is done atomically and that subscribers
     * are notified when the proxy's lifetime has ended.
     *
     * @post subscribers are notified after reference's lifetime has ended.
     * @return A reference to the shared value.
     */
    proxy_type proxy() noexcept
    {
        hi_axiom(_pimpl);
        return _pimpl->proxy();
    }

    /** Dereference to the value.
     *
     * @return A const reference to the value.
     */
    const_reference operator*() const noexcept
    {
        return *const_proxy();
    }

    /** Member select.
    * 
    * Member selection is done recursive through a read-only proxy object.
    * 
    * @return A read-only proxy object used to recursively member select.
    */
    const_proxy_type operator->() const noexcept
    {
        return const_proxy();
    }

    /** Construct an observable with its value set.
     *
     * @param value The value to assign to the shared-value.
     */
    observable(std::convertible_to<value_type> auto&& value) noexcept : _pimpl(std::make_shared<impl_type>(hi_forward(value)))
    {
        _pimpl->add_owner(*this);
    }

    /** Assign a new value.
     *
     * @post subscribers are notified.
     * @param value The value to assign to the shared-value.
     */
    observable& operator=(std::convertible_to<value_type> auto&& value) noexcept
    {
        hi_axiom(_pimpl);
        *proxy() = hi_forward(value);
        return *this;
    }

    /** Pre increment.
     *
     * Short for `++*lhs.proxy()`
     *
     * @param rhs The right hand side value.
     * @return The return value of the value_type::operator++().
     */
    auto operator++() noexcept requires(requires(value_type a) { {++a}; })
    {
        return ++*proxy();
    }

    /** Pre decrement.
     *
     * Short for `--*lhs.proxy()`
     *
     * @param rhs The right hand side value.
     * @return The return value of the value_type::operator--().
     */
    auto operator--() noexcept requires(requires(value_type a) { {--a}; })
    {
        return --*proxy();
    }

    /** Post increment.
     *
     * Short for `(*lhs.proxy())++`
     *
     * @param rhs The right hand side value.
     * @return The return value of the value_type::operator++(int).
     */
    auto operator++(int) noexcept requires(requires(value_type a) { {a++}; })
    {
        return (*proxy())++;
    }

    /** Post decrement.
     *
     * Short for `(*lhs.proxy())--`
     *
     * @param rhs The right hand side value.
     * @return The return value of the value_type::operator--(int).
     */
    auto operator--(int) noexcept requires(requires(value_type a) { {a--}; })
    {
        return (*proxy())--;
    }

    /** Inplace add.
     *
     * Short for `*lhs.proxy() += rhs`
     *
     * @param rhs The right hand side value.
     * @return The return value of the value_type::operator+=(rhs).
     */
    auto operator+=(auto&& rhs) noexcept requires(requires(value_type a, decltype(rhs) b) { {a += hi_forward(b)}; })
    {
        return *proxy() += hi_forward(rhs);
    }

    /** Inplace subtract.
     *
     * Short for `*lhs.proxy() -= rhs`
     *
     * @param rhs The right hand side value.
     * @return The return value of the value_type::operator-=(rhs).
     */
    auto operator-=(auto&& rhs) noexcept requires(requires(value_type a, decltype(rhs) b) { {a -= hi_forward(b)}; })
    {
        return *proxy() -= hi_forward(rhs);
    }

    /** Inplace multiply.
     *
     * Short for `*lhs.proxy() *= rhs`
     *
     * @param rhs The right hand side value.
     * @return The return value of the value_type::operator*=(rhs).
     */
    auto operator*=(auto&& rhs) noexcept requires(requires(value_type a, decltype(rhs) b) { {a *= hi_forward(b)}; })
    {
        return *proxy() *= hi_forward(rhs);
    }

    /** Inplace divide.
     *
     * Short for `*lhs.proxy() /= rhs`
     *
     * @param rhs The right hand side value.
     * @return The return value of the value_type::operator/=(rhs).
     */
    auto operator/=(auto&& rhs) noexcept requires(requires(value_type a, decltype(rhs) b) { {a /= hi_forward(b)}; })
    {
        return *proxy() /= hi_forward(rhs);
    }

    /** Inplace remainder.
     *
     * Short for `*lhs.proxy() %= rhs`
     *
     * @param rhs The right hand side value.
     * @return The return value of the value_type::operator%=(rhs).
     */
    auto operator%=(auto&& rhs) noexcept requires(requires(value_type a, decltype(rhs) b) { {a %= hi_forward(b)}; })
    {
        return *proxy() %= hi_forward(rhs);
    }

    /** Inplace bitwise and.
     *
     * Short for `*lhs.proxy() &= rhs`
     *
     * @param rhs The right hand side value.
     * @return The return value of the value_type::operator&=(rhs).
     */
    auto operator&=(auto&& rhs) noexcept requires(requires(value_type a, decltype(rhs) b) { {a &= hi_forward(b)}; })
    {
        return *proxy() &= hi_forward(rhs);
    }

    /** Inplace bitwise or.
     *
     * Short for `*lhs.proxy() |= rhs`
     *
     * @param rhs The right hand side value.
     * @return The return value of the value_type::operator|=(rhs).
     */
    auto operator|=(auto&& rhs) noexcept requires(requires(value_type a, decltype(rhs) b) { {a |= hi_forward(b)}; })
    {
        return *proxy() |= hi_forward(rhs);
    }

    /** Inplace bitwise xor.
     *
     * Short for `*lhs.proxy() ^= rhs`
     *
     * @param rhs The right hand side value.
     * @return The return value of the value_type::operator^=(rhs).
     */
    auto operator^=(auto&& rhs) noexcept requires(requires(value_type a, decltype(rhs) b) { {a ^= hi_forward(b)}; })
    {
        return *proxy() ^= hi_forward(rhs);
    }

    /** Inplace shift left.
     *
     * Short for `*lhs.proxy() <<= rhs`
     *
     * @param rhs The right hand side value.
     * @return The return value of the value_type::operator<<=(rhs).
     */
    auto operator<<=(auto&& rhs) noexcept requires(requires(value_type a, decltype(rhs) b) { {a <<= hi_forward(b)}; })
    {
        return *proxy() <<= hi_forward(rhs);
    }

    /** Inplace shift right.
     *
     * Short for `*lhs.proxy() >>= rhs`
     *
     * @param rhs The right hand side value.
     * @return The return value of the value_type::operator>>=(rhs).
     */
    auto operator>>=(auto&& rhs) noexcept requires(requires(value_type a, decltype(rhs) b) { {a >>= hi_forward(b)}; })
    {
        return *proxy() >>= hi_forward(rhs);
    }

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

} // namespace hi::inline v1
