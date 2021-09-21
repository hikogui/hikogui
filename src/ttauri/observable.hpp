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

/** Only scalar-observables should be atomic.
* 
* Non-scalars should be accessible by reference so that dereferencing works as expected.
*/
template<typename T>
constexpr bool observable_is_atomic_v = std::is_scalar_v<T> and sizeof(T) <= sizeof(ptrdiff_t);


template<typename T>
struct observable_impl;

template<typename T, bool Constant>
struct observable_proxy {
    using value_type = T;
    static constexpr bool is_atomic = observable_is_atomic_v<T>;
    static constexpr bool is_constant = Constant;
    static constexpr bool is_variable = not Constant;

    enum class state_type : uint8_t { none, read, write, modified };

    ~observable_proxy()
    {
        if (_actual) {
            if (is_variable and _state == state_type::write and _actual->value != _original) {
                _state = state_type::modified;
            }

            if (not is_atomic and _state != state_type::none) {
                _actual->mutex.unlock();
            }

            if (is_variable and _state == state_type::modified) {
                _actual->notify_owners();
            }
        }
    }

    observable_proxy(observable_impl<T> &actual) : _actual(&actual) {}

    observable_proxy(observable_proxy &&other) noexcept = delete;
    observable_proxy &operator=(observable_proxy &&other) noexcept = delete;
    observable_proxy(observable_proxy const &) = delete;
    observable_proxy &operator=(observable_proxy const &) = delete;

    void prepare(state_type state) const noexcept
    {
        tt_axiom(state != state_type::none);
        if (not is_atomic and _state == state_type::none) {
            this->_actual->mutex.lock();
        }
        if (_state <= state_type::read and state == state_type::write) {
            _original = _actual->value;
        }
        _state = state;
    }

    // MSVC Compiler bug returning this with auto argument
    template<std::convertible_to<value_type> Value>
    observable_proxy &operator=(Value &&value) noexcept requires(is_variable)
    {
        prepare(state_type::write);
        _actual->value = std::forward<decltype(value)>(value);
        return *this;
    }

    value_type operator*() const requires(is_atomic)
    {
        prepare(state_type::read);
        return _actual->value.load();
    }

    value_type const &operator*() const requires(is_constant and not is_atomic)
    {
        prepare(state_type::read);
        return _actual->value;
    }

    value_type &operator*() const requires(is_variable and not is_atomic)
    {
        prepare(state_type::write);
        return _actual->value;
    }

    value_type const *operator->() const requires(is_constant and not is_atomic)
    {
        prepare(state_type::read);
        return &(_actual->value);
    }

    value_type *operator->() const requires(is_variable and not is_atomic)
    {
        prepare(state_type::write);
        return &(_actual->value);
    }

    explicit operator bool() const noexcept
    {
        prepare(state_type::read);
        return static_cast<bool>(_actual->value);
    }

    template<typename... Args>
    auto operator()(Args &&...args) const noexcept
    {
        prepare(is_constant ? state_type::read : state_type::write);
        return _actual->value(std::forward<Args>(args)...);
    }

#define X(op) \
    [[nodiscard]] friend auto operator op(observable_proxy const &lhs, observable_proxy const &rhs) noexcept \
    { \
        lhs.prepare(state_type::read); \
        rhs.prepare(state_type::read); \
        return lhs._actual->value op rhs._actual->value; \
    } \
\
    [[nodiscard]] friend auto operator op(observable_proxy const &lhs, auto const &rhs) noexcept \
    { \
        lhs.prepare(state_type::read); \
        return lhs._actual->value op rhs; \
    } \
\
    [[nodiscard]] friend auto operator op(auto const &lhs, observable_proxy const &rhs) noexcept \
    { \
        rhs.prepare(state_type::read); \
        return lhs op rhs._actual->value; \
    }

    X(==)
    X(-)
#undef X

#define X(op) \
    [[nodiscard]] auto operator op() const noexcept \
    { \
        prepare(state_type::read); \
        return op _actual->value; \
    }

    X(-)
#undef X

    // MSVC Internal compiler error
    template<typename Rhs>
    value_type operator+=(Rhs const &rhs) noexcept requires(is_variable and std::is_arithmetic_v<Rhs>)
    {
        if (rhs != Rhs{}) {
            prepare(state_type::modified);
            return _actual->value += rhs;
        } else {
            prepare(state_type::read);
            return _actual->value;
        }
    }

    template<typename Rhs>
    value_type operator+=(Rhs const &rhs) noexcept requires(is_variable and not std::is_arithmetic_v<Rhs>)
    {
        prepare(state_type::write);
        return _actual->value += rhs;
    }

    observable_impl<value_type> *_actual = nullptr;
    mutable value_type _original;
    mutable state_type _state = state_type::none;
};

template<typename T>
struct observable_impl {
    static constexpr bool is_atomic = observable_is_atomic_v<T>;

    using value_type = T;
    using atomic_value_type = std::conditional_t<is_atomic, std::atomic<value_type>, value_type>;
    using owner_type = observable<value_type>;

    atomic_value_type value;
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

    owner_type *get_owner(size_t index) const noexcept
    {
        ttlet lock = std::scoped_lock(mutex);
        return index < owners.size() ? owners[index] : nullptr;
    }

    void notify_owners() const noexcept
    {
        size_t index = 0;
        while (auto owner = get_owner(index++)) {
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

/** An observable notifies listeners of changed to its value.
 *
 * An observable can be changed to other observers by assigning them to each other.
 *
 */
template<typename T>
class observable {
public:
    using value_type = T;
    using reference = detail::observable_proxy<value_type, false>;
    using const_reference = detail::observable_proxy<value_type, true>;
    using impl_type = detail::observable_impl<value_type>;
    using notifier_type = tt::notifier<void()>;
    using callback_ptr_type = notifier_type::callback_ptr_type;
    static constexpr bool is_atomic = impl_type::is_atomic;

    ~observable()
    {
        pimpl->remove_owner(this);
    }

    /** Construct an observerable.
     *
     * The observer is created with a value that is default constructed.
     */
    observable() noexcept : pimpl(std::make_shared<impl_type>())
    {
        pimpl->add_owner(this);
    }

    /** Construct an observerable and chain it to another.
     *
     * The new observable will share a value with the other observable.
     *
     * @param other The other observable to share the value with.
     */
    observable(observable const &other) noexcept : pimpl(other.pimpl)
    {
        pimpl->add_owner(this);
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

        pimpl->reseat_owners(other.pimpl);
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
        return pimpl->cget();
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
        return pimpl->cget();
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
        return pimpl->get();
    }

    /** Construct an observable with its value set.
     *
     * @param value The value to assign to the shared-value.
     */
    observable(std::convertible_to<value_type> auto &&value) noexcept :
        pimpl(std::make_shared<impl_type>(std::forward<decltype(value)>(value)))
    {
        pimpl->add_owner(this);
    }

    // XXX MSVC-2019 Compiler bug returning this with auto argument
    /** Assign a new value.
     *
     * @post subscribers are notified.
     * @param value The value to assign to the shared-value.
     */
    template<std::convertible_to<value_type> Value>
    observable &operator=(Value &&value) noexcept
    {
        tt_axiom(pimpl);
        get() = std::forward<decltype(value)>(value);
        return *this;
    }

    /** Get copy of the shared-value.
     *
     * @return a copy of the shared-value.
     */
    value_type operator*() const noexcept
    {
        return *(cget());
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
