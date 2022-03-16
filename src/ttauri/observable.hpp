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

template<typename T>
struct observable_impl;

/** A proxy to the shared-value inside an observable.
 *
 * This proxy object makes sure that updates to a value
 * is atomic and that modifications causes notifications to be
 * sent.
 *
 * @tparam T The type of the observed value.
 * @tparam Constant True when there is read-only access to the value. False if there is read-write access.
 */
template<typename T, bool Constant>
struct observable_proxy {
    using value_type = T;
    static constexpr bool is_constant = Constant;
    static constexpr bool is_variable = not Constant;

    enum class state_type : uint8_t { read, write, modified };

    observable_impl<value_type> *_actual = nullptr;
    mutable value_type _original;
    mutable state_type _state = state_type::read;

    ~observable_proxy()
    {
        tt_axiom(_actual);

        if (is_variable and _state == state_type::write and _actual->value != _original) {
            _state = state_type::modified;
        }

        if (is_variable and _state == state_type::modified) {
            _actual->notify_owners();
        }
    }

    observable_proxy(observable_impl<T> &actual) : _actual(&actual), _original(), _state(state_type::read)
    {
        tt_axiom(this->_actual);
    }

    observable_proxy(observable_proxy &&) = delete;
    observable_proxy(observable_proxy const &) = delete;

    void prepare_write() const noexcept
    {
        if (_state == state_type::read) {
            _original = _actual->value;
            _state = state_type::write;
        }
    }

    operator value_type &() const noexcept requires(is_variable)
    {
        prepare_write();
        return _actual->value;
    }

    operator value_type const &() const noexcept requires(is_constant)
    {
        return _actual->value;
    }

    /** Copy the value from a temporary proxy.
     */
    observable_proxy &operator=(observable_proxy &&other) noexcept
    {
        tt_return_on_self_assignment(other);

        return *this = other._actual->value;
    }

    /** Copy the value from a proxy.
     */
    observable_proxy &operator=(observable_proxy const &other) noexcept
    {
        tt_return_on_self_assignment(other);

        return *this = other._actual->value;
    }

    // MSVC Compiler bug returning this with auto argument
    template<typename Arg = value_type>
    observable_proxy &operator=(Arg &&arg) noexcept
        requires(is_variable and not std::is_same_v<std::remove_cvref_t<Arg>, observable_proxy>)
    {
        prepare_write();
        _actual->value = std::forward<Arg>(arg);
        return *this;
    }

    value_type const &operator*() const noexcept requires(is_constant)
    {
        return _actual->value;
    }

    value_type &operator*() const noexcept requires(is_variable)
    {
        prepare_write();
        return _actual->value;
    }

    value_type const *operator->() const noexcept requires(is_constant)
    {
        return &(_actual->value);
    }

    value_type *operator->() const noexcept requires(is_variable)
    {
        prepare_write();
        return &(_actual->value);
    }

    template<typename... Args>
    decltype(auto) operator()(Args &&...args) const noexcept
    {
        prepare_write();
        return _actual->value(std::forward<Args>(args)...);
    }

    template<typename Arg>
    decltype(auto) operator[](Arg &&arg) const noexcept requires(is_variable)
    {
        prepare_write();
        return _actual->value[std::forward<Arg>(arg)];
    }

    template<typename Arg>
    decltype(auto) operator[](Arg &&arg) const noexcept requires(is_constant)
    {
        return const_cast<value_type const &>(_actual->value)[std::forward<Arg>(arg)];
    }

    auto operator++() noexcept requires(is_variable and pre_incrementable<decltype(_actual->value)>)
    {
        _state = state_type::modified;
        return ++(_actual->value);
    }

    auto operator--() noexcept requires(is_variable and pre_decrementable<decltype(_actual->value)>)
    {
        _state = state_type::modified;
        return --(_actual->value);
    }

#define X(op) \
    [[nodiscard]] friend auto operator op(observable_proxy const &lhs, observable_proxy const &rhs) noexcept \
    { \
        return lhs._actual->value op rhs._actual->value; \
    } \
\
    [[nodiscard]] friend auto operator op(observable_proxy const &lhs, auto const &rhs) noexcept \
    { \
        return lhs._actual->value op rhs; \
    } \
\
    [[nodiscard]] friend auto operator op(auto const &lhs, observable_proxy const &rhs) noexcept \
    { \
        return lhs op rhs._actual->value; \
    }

    X(==)
    X(<=>)
    X(-)
    X(+)
    X(*)
    X(/)
    X(%)
    X(&)
    X(|)
#undef X

#define X(op) \
    [[nodiscard]] auto operator op() const noexcept \
    { \
        return op _actual->value; \
    }

    X(-)
    X(~)
#undef X

#define X(op) \
    value_type operator op(auto const &rhs) noexcept requires(is_variable) \
    { \
        prepare_write(); \
        return _actual->value op rhs; \
    }

    X(+=)
    X(-=)
#undef X

#define X(func) \
    [[nodiscard]] friend decltype(auto) func(observable_proxy const &rhs) noexcept \
    { \
        return func(rhs._actual->value); \
    }

    X(size)
    X(ssize)
    X(begin)
    X(end)
#undef X
};

/** The shared value, shared between observers.
 */
template<typename T>
struct observable_impl {
    using value_type = T;
    using owner_type = observable<value_type>;

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
            owner->_notifier();
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
            owner->_notifier();
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

    auto subscribe(std::invocable<> auto &&callback) noexcept
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
    tt::notifier<void()> _notifier;
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
