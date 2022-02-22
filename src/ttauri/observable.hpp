// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unfair_mutex.hpp"
#include "concepts.hpp"
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

inline unfair_mutex observable_mutex;

struct observable_notifier_type {
    using callback_ptr_type = std::weak_ptr<std::function<void()>>;

    uint64_t proxy_count = 0;
    std::vector<callback_ptr_type> callbacks;

    observable_notifier_type() = default;
    observable_notifier_type(observable_notifier_type const &) = delete;
    observable_notifier_type(observable_notifier_type &&) = delete;
    observable_notifier_type &operator=(observable_notifier_type const &) = delete;
    observable_notifier_type &operator=(observable_notifier_type &&) = delete;

    void start_proxy() noexcept
    {
        ttlet lock = std::scoped_lock(observable_mutex);
        ++proxy_count;
    }

    void finish_proxy() noexcept
    {
        observable_mutex.lock();

        tt_axiom(proxy_count != 0);
        --proxy_count;

        notify();
    }

    /** Add a callback to the list.
     *
     * @pre observable_mutex must be locked.
     */
    void push(callback_ptr_type callback_ptr) noexcept
    {
        tt_axiom(observable_mutex.is_locked());
        callbacks.push_back(std::move(callback_ptr));
    }

    /** Notify all callbacks that have been pushed.
     *
     * @pre observable_mutex must be locked.
     * @post obervable_mutex is unlocked
     * @post All registered callbacks have been called.
     * @post List of registered callbacks has been cleared.
     */
    void notify() noexcept
    {
        tt_axiom(observable_mutex.is_locked());

        if (proxy_count == 0 and not callbacks.empty()) {
            auto to_call = std::vector<callback_ptr_type>{};
            std::swap(to_call, callbacks);
            observable_mutex.unlock();

            for (ttlet &callback_ptr : to_call) {
                if (auto callback = callback_ptr.lock()) {
                    (*callback)();
                }
            }
        } else {
            observable_mutex.unlock();
        }
    }
};

inline observable_notifier_type observable_notifier;

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
    static constexpr bool is_atomic = std::is_scalar_v<T>;
    static constexpr bool is_constant = Constant;
    static constexpr bool is_variable = not Constant;

    enum class state_type : uint8_t { read, write, modified };

    observable_impl<value_type> *_actual = nullptr;
    mutable value_type _original;
    mutable state_type _state = state_type::none;

    ~observable_proxy()
    {
        tt_axiom(_actual);

        if (is_variable and _state == state_type::write and _actual->value != _original) {
            _state = state_type::modified;
        }

        if (not is_atomic) {
            _actual->mutex.unlock();
            observable_notifier.finish_proxy();
        }

        if (is_variable and _state == state_type::modified) {
            _actual->notify_owners();
        }
    }

    observable_proxy(observable_impl<T> &actual) : _actual(&actual), _original(), _state(state_type::read)
    {
        if (not is_atomic) {
            observable_notifier.start_proxy();
            this->_actual->mutex.lock();
        }
        tt_axiom(this->_actual);
    }

    observable_proxy(observable_proxy &&) = delete;
    observable_proxy(observable_proxy const &) = delete;

    void prepare(state_type new_state) const noexcept
    {
        tt_axiom(new_state != state_type::read);
        if (_state == state_type::read and new_state == state_type::write) {
            _original = _actual->value;
        }
        _state = new_state;
    }

    operator value_type() const noexcept requires(is_atomic)
    {
        return _actual->value;
    }

    operator value_type &() const noexcept requires(is_variable and not is_atomic)
    {
        prepare(state_type::write);
        return _actual->value;
    }

    operator value_type const &() const noexcept requires(is_constant and not is_atomic)
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
        requires(is_variable and not is_atomic and not std::is_same_v<std::remove_cvref_t<Arg>, observable_proxy>)
    {
        prepare(state_type::write);
        _actual->value = std::forward<Arg>(arg);
        return *this;
    }

    template<typename Arg = value_type>
    observable_proxy &operator=(Arg &&arg) noexcept
        requires(is_variable and is_atomic and not std::is_same_v<std::remove_cvref_t<Arg>, observable_proxy>)
    {
        // value is not std::forwarded, that means exchange() and operator!=() are called
        // with a const lvalue reference.
        if (_actual->value.exchange(arg) != arg) {
            _state = state_type::modified;
        }
        return *this;
    }

    value_type operator*() const noexcept requires(is_atomic)
    {
        return _actual->value.load();
    }

    value_type const &operator*() const noexcept requires(is_constant and not is_atomic)
    {
        return _actual->value;
    }

    value_type &operator*() const noexcept requires(is_variable and not is_atomic)
    {
        prepare(state_type::write);
        return _actual->value;
    }

    value_type const *operator->() const noexcept requires(is_constant and not is_atomic)
    {
        return &(_actual->value);
    }

    value_type *operator->() const noexcept requires(is_variable and not is_atomic)
    {
        prepare(state_type::write);
        return &(_actual->value);
    }

    template<typename... Args>
    decltype(auto) operator()(Args &&...args) const noexcept
    {
        prepare(state_type::write);
        return _actual->value(std::forward<Args>(args)...);
    }

    template<typename Arg>
    decltype(auto) operator[](Arg &&arg) const noexcept requires(is_variable and not is_atomic)
    {
        prepare(state_type::write);
        return _actual->value[std::forward<Arg>(arg)];
    }

    template<typename Arg>
    decltype(auto) operator[](Arg &&arg) const noexcept requires(is_constant and not is_atomic)
    {
        return const_cast<value_type const &>(_actual->value)[std::forward<Arg>(arg)];
    }

    auto operator++() noexcept requires(is_variable and is_atomic and not pre_incrementable<decltype(_actual->value)>)
    {
        auto expected_value = _actual->value.load();
        decltype(expected_value) new_value;
        do {
            // Make a copy so that expected value does not get incremented.
            new_value = expected_value;
            if (++new_value == expected_value) {
                return new_value;
            }
        } while (not _actual->value.compare_exchange_weak(expected_value, new_value));

        prepare(state_type::modified);
        return new_value;
    }

    auto operator--() noexcept requires(is_variable and is_atomic and not pre_decrementable<decltype(_actual->value)>)
    {
        auto expected_value = _actual->value.load();
        decltype(expected_value) new_value;
        do {
            // Make a copy so that expected value does not get incremented.
            new_value = expected_value;
            if (--new_value == expected_value) {
                return new_value;
            }
        } while (not _actual->value.compare_exchange_weak(expected_value, new_value));

        prepare(state_type::modified);
        return new_value;
    }

    auto operator++() noexcept requires(is_variable and is_atomic and pre_incrementable<decltype(_actual->value)>)
    {
        prepare(state_type::modified);
        return ++(_actual->value);
    }

    auto operator--() noexcept requires(is_variable and is_atomic and pre_decrementable<decltype(_actual->value)>)
    {
        prepare(state_type::modified);
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
    template<typename Rhs> \
    value_type operator op(Rhs const &rhs) noexcept requires(is_variable and std::is_arithmetic_v<Rhs>) \
    { \
        if (rhs != Rhs{}) { \
            prepare(state_type::modified); \
            return _actual->value op rhs; \
        } else { \
            return _actual->value; \
        } \
    } \
\
    template<typename Rhs> \
    value_type operator op(Rhs const &rhs) noexcept requires(is_variable and not std::is_arithmetic_v<Rhs>) \
    { \
        prepare(state_type::write); \
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

template<typename T>
struct observable_impl_storage {
    T value;

    observable_impl_storage() noexcept : value() {}
    observable_impl_storage(T const &value) noexcept : value(value) {}
    observable_impl_storage(T &&value) noexcept : value(std::move(value)) {}
};

template<scalar T>
struct observable_impl_storage<T> {
    std::atomic<T> value;

    observable_impl_storage() noexcept : value() {}
    observable_impl_storage(T const &value) noexcept : value(value) {}
    observable_impl_storage(T &&value) noexcept : value(std::move(value)) {}
};

/** The shared value, shared between observers.
 */
template<typename T>
struct observable_impl : public observable_impl_storage<T> {
    using value_type = T;
    using owner_type = observable<value_type>;

    std::vector<owner_type *> owners;

    /** The mutex is used to serialize state to the owners list and non-atomic value.
     */
    mutable unfair_mutex mutex;

    ~observable_impl()
    {
        tt_axiom(owners.empty());
    }

    observable_impl(observable_impl const &) = delete;
    observable_impl(observable_impl &&) = delete;
    observable_impl &operator=(observable_impl const &) = delete;
    observable_impl &operator=(observable_impl &&) = delete;

    observable_impl() noexcept : observable_impl_storage<T>() {}
    observable_impl(value_type const &value) noexcept : observable_impl_storage<T>(value) {}
    observable_impl(value_type &&value) noexcept : observable_impl_storage<T>(std::move(value)) {}

    observable_proxy<value_type, false> get() noexcept
    {
        return observable_proxy<value_type, false>(*this);
    }

    observable_proxy<value_type, true> cget() noexcept
    {
        return observable_proxy<value_type, true>(*this);
    }

    /** Add an observer as one of the owners of the shared-value.
     *
     * @param owner A reference to observer
     */
    void add_owner(owner_type &owner) noexcept
    {
        tt_axiom(std::find(owners.cbegin(), owners.cend(), &owner) == owners.cend());

        ttlet lock = std::scoped_lock(observable_mutex);
        owners.push_back(&owner);
    }

    /** Remove an observer as one of the owners of the shared-value.
     *
     * @param owner A reference to observer
     */
    void remove_owner(owner_type &owner) noexcept
    {
        ttlet lock = std::scoped_lock(observable_mutex);
        ttlet nr_erased = std::erase(owners, &owner);
        tt_axiom(nr_erased == 1);
    }

    void notify_owners() const noexcept
    {
        observable_mutex.lock();
        for (auto owner : owners) {
            owner->notify();
        }
        observable_notifier.notify();
    }

    void reseat_owners(std::shared_ptr<observable_impl> const &new_impl) noexcept
    {
        observable_mutex.lock();

        tt_axiom(not owners.empty());
        auto keep_this_alive = owners.front()->_pimpl;

        for (auto owner : owners) {
            owner->_pimpl = new_impl;
            new_impl->owners.push_back(owner);
            owner->notify();
        }
        owners.clear();
        observable_notifier.notify();
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
 * When assigning observables to each other, the subscriptions
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
 * A subscription to a observable is maintained using a `std::weak_ptr`
 * to a callback function. This means that the object that subscribed
 * to the observable needs to own the `std::shared_ptr` that is returned
 * by the `subscribe()` method.
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
    using callback_ptr_type = std::shared_ptr<std::function<void()>>;
    static constexpr bool is_atomic = std::is_scalar_v<value_type>;

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
    observable(observable const &other) noexcept : _pimpl(other._pimpl)
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
    observable &operator=(observable const &other) noexcept
    {
        tt_return_on_self_assignment(other);

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
        return _pimpl->get();
    }

    /** Construct an observable with its value set.
     *
     * @param value The value to assign to the shared-value.
     */
    observable(std::convertible_to<value_type> auto &&value) noexcept :
        _pimpl(std::make_shared<impl_type>(std::forward<decltype(value)>(value)))
    {
        _pimpl->add_owner(*this);
    }

    /** Assign a new value.
     *
     * @post subscribers are notified.
     * @param value The value to assign to the shared-value.
     */
    template<typename Value = value_type>
    observable &operator=(Value &&value) noexcept requires(not std::is_same_v<std::remove_cvref_t<Value>, observable>)
    {
        tt_axiom(_pimpl);
        get() = std::forward<Value>(value);
        return *this;
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

    callback_ptr_type subscribe(callback_ptr_type const &callback_ptr) noexcept
    {
        ttlet lock = std::scoped_lock(detail::observable_mutex);
#if TT_BUILD_TYPE == TT_BT_DEBUG
        auto it = std::find_if(_callbacks.cbegin(), _callbacks.cend(), [&callback_ptr](ttlet &item) {
            return item.lock() == callback_ptr;
        });
        tt_axiom(it == _callbacks.cend());
#endif
        _callbacks.push_back(callback_ptr);
        return callback_ptr;
    }

    template<typename Callback>
    [[nodiscard]] callback_ptr_type subscribe(Callback &&callback) noexcept requires(std::is_invocable_v<Callback>)
    {
        auto callback_ptr = std::make_shared<std::function<void()>>(std::forward<Callback>(callback));
        subscribe(callback_ptr);

        detail::observable_mutex.lock();
        detail::observable_notifier.push(callback_ptr);
        detail::observable_notifier.notify();
        return callback_ptr;
    }

    void unsubscribe(callback_ptr_type const &callback_ptr) noexcept
    {
        ttlet lock = std::scoped_lock(detail::observable_mutex);
        ttlet erase_count = std::erase_if(_callbacks, [&callback_ptr](ttlet &item) {
            return item.expired() or item.lock() == callback_ptr;
        });
        tt_axiom(erase_count == 1);
    }

private:
    std::shared_ptr<impl_type> _pimpl;
    mutable std::vector<std::weak_ptr<std::function<void()>>> _callbacks;

    void notify() const noexcept
    {
        auto has_expired_callbacks = false;
        for (ttlet &callback_ptr : _callbacks) {
            if (callback_ptr.expired()) {
                has_expired_callbacks = true;
            } else {
                detail::observable_notifier.push(callback_ptr);
            }
        }

        if (has_expired_callbacks) {
            ttlet erase_count = std::erase_if(_callbacks, [](ttlet &callback_ptr) {
                return callback_ptr.expired();
            });
            tt_axiom(erase_count > 0);
        }
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

} // namespace tt::inline v1
