// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "rcu.hpp"
#include "notifier.hpp"
#include "tree.hpp"
#include "concepts.hpp"
#include <string_view>
#include <string>
#include <memory>

namespace hi::inline v1 {

namespace detail {
template<typename, bool>
class observer;
}

/** An abstract observable object.
 *
 * This type is referenced by `observer`s
 */
class observable : public std::enable_shared_from_this<observable> {
private:
    /** The type of the notifier used to notify changes to the value of the observable.
     */
    using notifier_type = notifier<void(void const *, void const *)>;

public:
    /** The token returned by `subscribe()`.
     */
    using token_type = notifier_type::token_type;

    /** The type of the callback that can be subscribed.
     */
    using function_proto = notifier_type::function_proto;

    /** The type of the path used for notifying observers.
     */
    using path_type = std::vector<std::string>;

    constexpr virtual ~observable() = default;
    observable(observable const&) = delete;
    observable(observable&&) = delete;
    observable& operator=(observable const&) = delete;
    observable& operator=(observable&&) = delete;
    constexpr observable() noexcept = default;

    /** Get a pointer to the current value.
     *
     * @note `read()` does not `read_lock()` the observable and should be done before `read()`.
     * @return A const pointer to the value. The `observer` should cast this to a pointer to the value-type.
     */
    [[nodiscard]] virtual void const *read() const noexcept = 0;

    /** Allocate and make a copy of the value.
     *
     * @note `copy()` does not `write_lock()` the observable and should be done before `read()`.
     * @param A pointer to the value that was `read()`.
     * @return A pointer to a newly allocated copy of the value.
     */
    [[nodiscard]] virtual void *copy(void const *ptr) const noexcept = 0;

    /** Commit the modified copy.
     *
     * @note `commit()` does not `write_unlock()`.
     * @param ptr A pointer to the modified new value returned by `copy()`.
     */
    virtual void commit(void *ptr) noexcept = 0;

    /** Abort the modified copy.
     *
     * @note `abort()` does not `write_unlock()`.
     * @param ptr A pointer to the modified new value returned by `copy()`.
     */
    virtual void abort(void *ptr) const noexcept = 0;

    /** Lock for reading.
     */
    virtual void read_lock() const noexcept = 0;

    /** Unlock for reading.
     */
    virtual void read_unlock() const noexcept = 0;

    /** Lock for writing.
     */
    virtual void write_lock() const noexcept = 0;

    /** Unlock for writing.
     */
    virtual void write_unlock() const noexcept = 0;

    /** Subscribe a callback with the observer.
     *
     * @param path The path within the observable-value that being watched.
     * @param flags The way the callback should be called.
     * @param function The function to be called when the watched observable-value changes.
     *                 The function has two `void *` arguments to the old value and the new value.
     *                 It is the task of the observer to cast the `void *` to the actual value-type.
     * @return A token which will extend the lifetime of the function. When the token is destroyed
     *         the function will be unsubscribed.
     */
    [[nodiscard]] token_type
    subscribe(path_type const& path, callback_flags flags, forward_of<function_proto> auto&& function) noexcept
    {
        auto& notifier = _notifiers[path];
        return notifier.subscribe(flags, hi_forward(function));
    }

    /** Called by a observer to notify all observers that the value has changed.
     *
     * The @a path argument is used to determine which of the subscribed callback will be called.
     *  - All callbacks which are a prefix of @a path.
     *  - All callbacks which have @a path as a prefix.
     *
     * @param old_ptr The pointer to the old value.
     * @param new_ptr The pointer to the new value.
     * @param path The path of the observed-value that was modified.
     */
    void notify(void const *old_ptr, void const *new_ptr, path_type const& path) const noexcept
    {
        _notifiers.walk_including_path(path, [old_ptr, new_ptr](notifier_type const& notifier) {
            notifier(old_ptr, new_ptr);
        });
    }

private:
    tree<std::string, notifier_type> _notifiers;

    template<typename, bool>
    friend class detail::observer;
};

template<typename T>
class observable_value final : public observable {
public:
    using value_type = T;

    ~observable_value() = default;

    /** Construct the shared state and default initialize the value.
     *
     * @param args The arguments passed to the constructor of the value.
     */
    constexpr observable_value() noexcept : _rcu()
    {
        _rcu.emplace(value_type{});
    }

    /** Construct the shared state and initialize the value.
     *
     * @param args The arguments passed to the constructor of the value.
     */
    template<typename... Args>
    constexpr observable_value(Args&&...args) noexcept : _rcu()
    {
        _rcu.emplace(std::forward<Args>(args)...);
    }

    /** Get a observer to the value.
     *
     * This function returns a observer to the value object.
     * The observer is used to start read or write transactions or create other observers.
     *
     * @return The new observer pointing to the value object.
     */
    [[nodiscard]] detail::observer<value_type, true> observer() const& noexcept
    {
        return {std::const_pointer_cast<observable>(this->shared_from_this())};
    }

private:
    using path_type = observable::path_type;

    rcu<value_type> _rcu;
    mutable unfair_mutex _write_mutex;

    [[nodiscard]] void const *read() const noexcept override
    {
        return _rcu.get();
    }

    [[nodiscard]] void *copy(void const *ptr) const noexcept override
    {
        return _rcu.copy(static_cast<value_type const *>(ptr));
    }

    void commit(void *ptr) noexcept override
    {
        _rcu.commit(static_cast<value_type *>(ptr));
    }

    void abort(void *ptr) const noexcept override
    {
        _rcu.abort(static_cast<value_type *>(ptr));
    }

    void read_lock() const noexcept override
    {
        _rcu.lock();
    }

    void read_unlock() const noexcept override
    {
        _rcu.unlock();
    }

    void write_lock() const noexcept override
    {
        _write_mutex.lock();
        read_lock();
    }

    void write_unlock() const noexcept override
    {
        read_unlock();
        _write_mutex.unlock();
    }
};

/** Shared state of an application.
 *
 * The shared state of an application that can be manipulated by the GUI,
 * preference and other systems.
 *
 * A `observer` selects a member or indexed element from the shared state,
 * or from another observer. You can `.read()` or `.copy()` the value pointed to
 * by the observer to read and manipulate the shared-data.
 *
 * Both `.read()` and `.copy()` take the full shared-state as a whole not allowing
 * other threads to have write access to this reference or copy. A copy will be
 * automatically committed, or may be aborted as well.
 *
 * lifetime:
 * - The lifetime of `observer` will extend the lifetime of `shared_state`.
 * - The lifetime of `observer::proxy` must be within the lifetime of `observer`.
 * - The lifetime of `observer::const_proxy` must be within the lifetime of `observer`.
 * - Although `observer` are created from another `observer` they internally do not
 *   refer to each other so their lifetime are not connected.
 *
 * @tparam T type used as the shared state.
 */
template<typename T>
class shared_state {
public:
    using value_type = T;

    ~shared_state() = default;
    constexpr shared_state(shared_state const&) noexcept = default;
    constexpr shared_state(shared_state&&) noexcept = default;
    constexpr shared_state& operator=(shared_state const&) noexcept = default;
    constexpr shared_state& operator=(shared_state&&) noexcept = default;

    /** Construct the shared state and initialize the value.
     *
     * @param args The arguments passed to the constructor of the value.
     */
    template<typename... Args>
    constexpr shared_state(Args&&...args) noexcept :
        _pimpl(std::make_shared<observable_value<value_type>>(std::forward<Args>(args)...))
    {
    }

    /** Get a observer to the value.
     *
     * This function returns a observer to the value object.
     * The observer is used to start read or write transactions or create other observers.
     *
     * @return The new observer pointing to the value object.
     */
    [[nodiscard]] detail::observer<value_type, true> observer() const noexcept
    {
        return _pimpl->observer();
    }

    // clang-format off
    /** Get a observer to a sub-object of value accessed by the index operator.
     *
     * @param index The index used with the index operator of the value.
     * @return The new observer pointing to a sub-object of the value.
     */
    [[nodiscard]] auto operator[](auto const& index) const& noexcept
        requires requires { observer()[index]; }
    {
        return observer()[index];
    }
    // clang-format on

    /** Get a observer to a member variable of the value.
     *
     * @note This requires the specialization of `hi::selector<value_type>`.
     * @tparam Name the name of the member variable of the value.
     * @return The new observer pointing to the member variable of the value
     */
    template<basic_fixed_string Name>
    [[nodiscard]] auto get() const& noexcept
    {
        return observer().get<Name>();
    }

private:
    std::shared_ptr<observable_value<value_type>> _pimpl;
};

namespace detail {

/** A observer pointing to the whole or part of a observable.
 *
 * A observer will point to a observable that was created, or possibly
 * an anonymous observable, which is created when a observer is created
 * as empty.
 *
 * @tparam T The type of observer.
 */
template<typename T, bool IsMutable>
class observer {
public:
    constexpr static bool is_mutable = IsMutable;

    using value_type = T;
    using notifier_type = notifier<void(value_type const&, value_type const&)>;
    using token_type = notifier_type::token_type;
    using function_proto = notifier_type::function_proto;
    using awaiter_type = notifier_type::awaiter_type;
    using path_type = observable::path_type;

    /** A proxy object of the observer.
     *
     * The proxy is a RAII object that manages a transaction with the
     * shared-state as a whole, while giving access to only a sub-object
     * of the shared-state.
     *
     * @tparam IsWriting The proxy is being used to write
     */
    template<bool IsWriting>
    class _proxy {
    public:
        constexpr static bool is_writing = IsWriting;

        using void_pointer = std::conditional_t<is_writing, void *, void const *>;
        using const_void_pointer = void const *;

        using reference = std::conditional_t<is_writing, value_type&, value_type const&>;
        using const_reference = value_type const&;
        using pointer = std::conditional_t<is_writing, value_type *, value_type const *>;
        using const_pointer = value_type const *;

        /** Commits and destruct the proxy object.
         *
         * If `commit()` or `abort()` are called or the proxy object
         * is empty then the destructor does not commit the changes.
         */
        ~_proxy() noexcept
        {
            _commit();
        }

        _proxy(_proxy const&) = delete;
        _proxy& operator=(_proxy const&) = delete;

        _proxy(_proxy const& other) noexcept requires(not is_writing) :
            _observer(other._observer), _old_base(nullptr), _new_base(other._new_base), _value(other._value)
        {
            _observer->read_lock();
        }

        _proxy& operator=(_proxy const& other) noexcept requires(not is_writing)
        {
            _commit();
            _observer = other._observer;
            _old_base = nullptr;
            _new_base = other._new_base;
            _value = other._value;
            _observer->read_lock();
            return *this;
        };

        _proxy(_proxy&& other) noexcept :
            _observer(std::exchange(other._observer, nullptr)),
            _old_base(std::exchange(other._old_base, nullptr)),
            _new_base(std::exchange(other._new_base, nullptr)),
            _value(std::exchange(other._value, nullptr))
        {
        }

        _proxy& operator=(_proxy&& other) noexcept
        {
            _commit();
            _observer = std::exchange(other._observer, nullptr);
            _old_base = std::exchange(other._old_base, nullptr);
            _new_base = std::exchange(other._new_base, nullptr);
            _value = std::exchange(other._value, nullptr);
            return *this;
        }

        /** Construct an empty proxy object.
         */
        constexpr _proxy() noexcept = default;

        /** Dereference the value.
         *
         * This function allows reads and modification to the value
         *
         * @note It is undefined behavior to call this function after calling `commit()` or `abort()`
         */
        reference operator*() noexcept
        {
            hi_axiom(_value != nullptr);
            return *_value;
        }

        /** Dereference the value.
         *
         * This function allows reads and modification to the value
         *
         * @note It is undefined behavior to call this function after calling `commit()` or `abort()`
         */
        const_reference operator*() const noexcept
        {
            hi_axiom(_value != nullptr);
            return *_value;
        }

        /** Pointer dereference the value.
         *
         * This function allows reads and modification to the value, including
         * calling member functions on the value.
         *
         * @note It is undefined behavior to call this function after calling `commit()` or `abort()`
         */
        pointer operator->() noexcept
        {
            hi_axiom(_value != nullptr);
            return _value;
        }

        /** Pointer dereference the value.
         *
         * This function allows reads and modification to the value, including
         * calling member functions on the value.
         *
         * @note It is undefined behavior to call this function after calling `commit()` or `abort()`
         */
        const_pointer operator->() const noexcept
        {
            hi_axiom(_value != nullptr);
            return _value;
        }

        /** Commit the changes to the value early.
         *
         * Calling this function allows to commit earlier than the destructor.
         *
         * @note It is undefined behavior to change the value after committing.
         */
        void commit() noexcept
        {
            _commit();
            _observer = nullptr;
#ifndef NDEBUG
            _value = nullptr;
#endif
        }

        /** Revert any changes to the value.
         *
         * Calling this function allows to abort any changes in the value.
         *
         * @note It is undefined behavior to change the value after aborting.
         */
        void abort() noexcept
        {
            _abort();
            _observer = nullptr;
#ifndef NDEBUG
            _value = nullptr;
#endif
        }

        // clang-format off

        // prefix operators
#define X(op) \
        template<typename Rhs> \
        decltype(auto) operator op() noexcept \
            requires is_writing and requires(value_type& a) { op a; } \
        { \
            return op (*_value); \
        }
    
        X(++)
        X(--)
#undef X
    
        // suffix operators
#define X(op) \
        template<typename Rhs> \
        auto operator op(int) noexcept \
            requires is_writing and requires(value_type& a) { a op; } \
        { \
            return (*_value) op; \
        }
    
        X(++)
        X(--)
#undef X
    
        // inplace operators
#define X(op) \
        template<typename Rhs> \
        decltype(auto) operator op(Rhs const& rhs) noexcept \
            requires is_writing and requires(value_type& a, Rhs const& b) { a op b; } \
        { \
            return (*_value) op rhs; \
        }
    
        X(+=)
        X(-=)
        X(*=)
        X(/=)
        X(%=)
        X(&=)
        X(|=)
        X(^=)
        X(<<=)
        X(>>=)
#undef X
    
        // mono operators
#define X(op) \
        template<typename Rhs> \
        auto operator op() const noexcept \
            requires requires(value_type const& a) { op a; } \
        { \
            return op (*_value); \
        }
    
        X(-)
        X(+)
        X(~)
        X(!)
#undef X
    
        // binary operators
#define X(op) \
        template<typename Rhs> \
        auto operator op(Rhs const& rhs) const noexcept \
            requires requires(value_type const& a, Rhs const& b) { a op b; } \
        { \
            return (*_value) op rhs; \
        }
    
        X(==)
        X(<=>)
        X(+)
        X(-)
        X(*)
        X(/)
        X(%)
        X(&)
        X(|)
        X(^)
        X(<<)
        X(>>)
#undef X
    
        // call operator
        template<typename... Args>
        auto operator()(Args &&... args) const noexcept
            requires requires(value_type const & a, Args &&...args) { a(std::forward<Args>(args)...); }
        {
            return (*_value)(std::forward<Args>(args)...);
        }
    
        template<typename... Args>
        auto operator()(Args &&... args) noexcept
            requires is_writing and requires(value_type & a, Args &&...args) { a(std::forward<Args>(args)...); }
        {
            return (*_value)(std::forward<Args>(args)...);
        }
        
        // index operator
        // XXX c++23
        template<typename Arg>
        auto operator[](Arg && arg) const noexcept
            requires requires(value_type const & a, Arg &&arg) { a[std::forward<Arg>(arg)]; }
        {
            return (*_value)[std::forward<Arg>(arg)];
        }

        template<typename Arg>
        auto operator[](Arg && arg) noexcept
            requires is_writing and requires(value_type & a, Arg &&arg) { a[std::forward<Arg>(arg)]; }
        {
            return (*_value)[std::forward<Arg>(arg)];
        }

        // clang-format on

    private:
        observer const *_observer = nullptr;
        const_void_pointer _old_base = nullptr;
        void_pointer _new_base = nullptr;
        pointer _value = nullptr;

        /** Create a proxy object.
         *
         * @param observer a pointer to the observer.
         * @param old_base A pointer to the old_base which holds the previous value used to create the new_base.
         *                 For a const_proxy this must be a nullptr.
         * @param new_base a pointer to the dereference rcu-object from the shared_state.
         *             This is needed to commit or abort the shared_state as a whole.
         * @param value a pointer to the sub-object of the shared_state that the observer
         *              is pointing to.
         */
        _proxy(observer const *observer, const_void_pointer old_base, void_pointer new_base, pointer value) noexcept :
            _observer(observer), _old_base(old_base), _new_base(new_base), _value(value)
        {
            hi_axiom(_observer != nullptr);
            if constexpr (is_writing) {
                hi_axiom(_old_base != nullptr);
            } else {
                hi_axiom(_old_base == nullptr);
            }
            hi_axiom(_new_base != nullptr);
            hi_axiom(_value != nullptr);
        }

        void _commit() noexcept
        {
            if (_observer != nullptr) {
                if constexpr (is_writing) {
                    _observer->commit(_old_base, _new_base);
                } else {
                    _observer->read_unlock();
                }
            }
        }

        void _abort() noexcept
        {
            if (_observer != nullptr) {
                if constexpr (is_writing) {
                    _observer->abort(_new_base);
                } else {
                    _observer->read_unlock();
                }
            }
        }

        friend class observer;
    };

    using proxy = std::conditional_t<is_mutable, _proxy<true>, void>;
    using const_proxy = _proxy<false>;

    constexpr ~observer() = default;

    /** Create an observer from an observable.
     *
     * @param observed The `observable` which will be observed by this observer.
     */
    observer(forward_of<std::shared_ptr<observable>> auto&& observed) noexcept :
        observer(hi_forward(observed), path_type{}, [](void *base) {
            return base;
        })
    {
    }

    /** Create a observer linked to an anonymous default initialized observed-value.
     */
    constexpr observer() noexcept : observer(std::make_shared<observable_value<value_type>>()) {}

    /** Create a observer linked to an anonymous observed-value.
     */
    constexpr observer(forward_of<value_type> auto&& value) noexcept :
        observer(std::make_shared<observable_value<value_type>>(hi_forward(value)))
    {
    }

    /** Copy construct.
     *
     * @note callback subscriptions are not copied.
     * @param other The other observer.
     */
    constexpr observer(observer const& other) noexcept : observer(other._observed, other._path, other._convert) {}

    /** Move construct.
     *
     * @note callback subscriptions are not copied.
     * @param other The other observer.
     */
    constexpr observer(observer&& other) noexcept :
        observer(std::move(other._observed), std::move(other._path), std::move(other._convert))
    {
        other.reset();
    }

    /** Copy construct.
     *
     * @note callback subscriptions are not copied.
     * @param other The other observer.
     */
    constexpr observer(observer<value_type, true> const& other) noexcept requires(not is_mutable) :
        observer(other._observed, other._path, other._convert)
    {
    }

    /** Move construct.
     *
     * @note callback subscriptions are not copied.
     * @param other The other observer.
     */
    constexpr observer(observer<value_type, true>&& other) noexcept requires(not is_mutable) :
        observer(std::move(other._observed), std::move(other._path), std::move(other._convert))
    {
        other.reset();
    }

    /** Copy assign.
     *
     * @note callback subscriptions remain unchanged and are not copied.
     * @param other The other observer.
     * @return this
     */
    constexpr observer& operator=(observer const& other) noexcept
    {
        // Get the old-value to notify with.
        _observed->read_lock();
        void const *old_base = _observed->read();
        value_type const *old_value = convert(old_base);

        // Replace the observer.
        auto old_observed = std::exchange(_observed, other._observed);
        _path = other._path;
        _convert = other._convert;

        // Get the new-value to notify with.
        _observed->read_lock();
        void const *new_base = _observed->read();
        value_type const *new_value = convert(new_base);

        // Rewire the callback subscriptions and notify listeners to this observer.
        update_state_callback();
        _notifier(*old_value, *new_value);

        _observed->read_unlock();
        old_observed->read_unlock();
        return *this;
    }

    /** Move assign.
     *
     * @note Callback subscriptions remain unchanged and are not moved.
     * @note The other shared observer will be attached to the anonymous state.
     * @param other The other observer.
     * @return this
     */
    constexpr observer& operator=(observer&& other) noexcept
    {
        // Get the old-value to notify with.
        _observed->read_lock();
        void const *old_base = _observed->read();
        value_type const *old_value = convert(old_base);

        // Replace the observer.
        auto old_observed = std::exchange(_observed, std::move(other._observed));
        _path = std::move(other._path);
        _convert = std::move(other._convert);
        other.reset();

        // Get the new-value to notify with.
        _observed->read_lock();
        void const *new_base = _observed->read();
        value_type const *new_value = convert(new_base);

        // Rewire the callback subscriptions and notify listeners to this observer.
        update_state_callback();
        _notifier(*old_value, *new_value);

        _observed->read_unlock();
        old_observed->read_unlock();
        return *this;
    }

    /** Copy assign.
     *
     * @note callback subscriptions remain unchanged and are not copied.
     * @param other The other observer.
     * @return this
     */
    constexpr observer& operator=(observer<value_type, true> const& other) noexcept requires(not is_mutable)
    {
        // Get the old-value to notify with.
        _observed->read_lock();
        void const *old_base = _observed->read();
        value_type const *old_value = convert(old_base);

        // Replace the observer.
        auto old_observed = std::exchange(_observed, other._observed);
        _path = other._path;
        _convert = other._convert;

        // Get the new-value to notify with.
        _observed->read_lock();
        void const *new_base = _observed->read();
        value_type const *new_value = convert(new_base);

        // Rewire the callback subscriptions and notify listeners to this observer.
        update_state_callback();
        _notifier(*old_value, *new_value);

        _observed->read_unlock();
        old_observed->read_unlock();
        return *this;
    }

    /** Move assign.
     *
     * @note Callback subscriptions remain unchanged and are not moved.
     * @note The other shared observer will be attached to the anonymous state.
     * @param other The other observer.
     * @return this
     */
    constexpr observer& operator=(observer<value_type, true>&& other) noexcept requires(not is_mutable)
    {
        // Get the old-value to notify with.
        _observed->read_lock();
        void const *old_base = _observed->read();
        value_type const *old_value = convert(old_base);

        // Replace the observer.
        auto old_observed = std::exchange(_observed, std::move(other._observed));
        _path = std::move(other._path);
        _convert = std::move(other._convert);
        other.reset();

        // Get the new-value to notify with.
        _observed->read_lock();
        void const *new_base = _observed->read();
        value_type const *new_value = convert(new_base);

        // Rewire the callback subscriptions and notify listeners to this observer.
        update_state_callback();
        _notifier(*old_value, *new_value);

        _observed->read_unlock();
        old_observed->read_unlock();
        return *this;
    }

    /** Reset the observer.
     *
     * This will link the observer with an anonymous observable with a default initialized value.
     */
    void reset() noexcept
    {
        _observed = std::make_shared<observable_value<value_type>>();
        _path = {};
        _convert = [](void *base) {
            return base;
        };
        update_state_callback();
    }

    /** Read the observed value.
     *
     * @return A const-proxy object used to access the data being observed.
     */
    [[nodiscard]] const_proxy read() const& noexcept
    {
        _observed->read_lock();
        auto new_base = _observed->read();
        return const_proxy{this, nullptr, new_base, convert(new_base)};
    }
    const_proxy read() && = delete;

    /** Make a copy of the observed value for modification.
     *
     * @return A proxy object used to modify the data being observed.
     */
    [[nodiscard]] proxy copy() const& noexcept requires(is_mutable)
    {
        _observed->write_lock();
        void const *old_base = _observed->read();
        void *new_base = _observed->copy(old_base);
        return proxy(this, old_base, new_base, convert(new_base));
    }
    proxy copy() && requires(is_mutable) = delete;

    /** Subscribe a callback to this observer.
     *
     * @param flags The callback flags on how to call the function.
     * @param function The function used as callback in the form `void(value_type const &old_value, value_type const &new_value)`
     * @return A callback-token used to extend the lifetime of the callback function.
     */
    [[nodiscard]] token_type subscribe(callback_flags flags, forward_of<function_proto> auto&& function) noexcept
    {
        return _notifier.subscribe(flags, hi_forward(function));
    }

    awaiter_type operator co_await() const noexcept
    {
        return _notifier.operator co_await();
    }

    /** Create a sub-observer by indexing into the value.
     *
     * @param index The index into the value being observed.
     * @return A new sub-observer which monitors the selected sub-value.
     */
    [[nodiscard]] auto get(auto const& index) const noexcept requires(requires() { std::declval<value_type>()[index]; })
    {
        using result_type = std::decay_t<decltype(std::declval<value_type>()[index])>;

        auto new_path = _path;
        new_path.push_back(std::format("[{}]", index));
        return detail::observer<result_type, is_mutable>{
            _observed, std::move(new_path), [convert_copy = this->_convert, index](void *base) -> void * {
                return std::addressof((*std::launder(static_cast<value_type *>(convert_copy(base))))[index]);
            }};
    }

    /** Create a sub-observer by selecting a member-variable of the value.
     *
     * @note This function requires the `hi::selector` type-trait to be implemented for `value_type`.
     * @tparam Name The name of the member-variable of value.
     * @return A new sub-observer which monitors the member of value.
     */
    template<basic_fixed_string Name>
    [[nodiscard]] auto get() const noexcept
    {
        using result_type = std::decay_t<decltype(selector<value_type>{}.get<Name>(std::declval<value_type&>()))>;

        auto new_path = _path;
        new_path.push_back(std::string{Name});
        // clang-format off
        return detail::observer<result_type, is_mutable>(
            _observed, std::move(new_path), [convert_copy = this->_convert](void *base) -> void * {
                return std::addressof(selector<value_type>{}.get<Name>(
                    *std::launder(static_cast<value_type *>(convert_copy(base)))));
            });
        // clang-format on
    }

    //
    // Convenient functions / operators working on temporary proxies.
    //

    /** Copy-assign a new value to the observed value.
     */
    observer& operator=(value_type const& rhs) noexcept requires(is_mutable)
    {
        *copy() = rhs;
        return *this;
    }

    /** Move-assign a new value to the observed value.
     */
    observer& operator=(value_type&& rhs) noexcept requires(is_mutable)
    {
        *copy() = std::move(rhs);
        return *this;
    }

    /** Get a copy of the value being observed.
     */
    value_type operator*() const noexcept
    {
        // This returns a copy of the dereferenced value of the proxy.
        // The proxy's lifetime will be extended for the copy to be made.
        return *read();
    }

    /** Constant pointer-to-member of the value being observed.
     */
    const_proxy operator->() const& noexcept
    {
        return read();
    }
    const_proxy operator->() && = delete;

    // clang-format off

    // prefix operators
#define X(op) \
    template<typename Rhs> \
    observer& operator op() noexcept \
        requires is_mutable and requires(value_type& a) { op a; } \
    { \
        op *copy(); \
        return *this; \
    }

    X(++)
    X(--)
#undef X

    // suffix operators
#define X(op) \
    template<typename Rhs> \
    auto operator op(int) noexcept \
        requires is_mutable and requires(value_type& a) { a op; } \
    { \
        return *copy() op; \
    }

    X(++)
    X(--)
#undef X

    // inplace operators
#define X(op) \
    template<typename Rhs> \
    observer& operator op(Rhs const& rhs) noexcept \
        requires is_mutable and requires(value_type& a, Rhs const& b) { a op b; } \
    { \
        *copy() op rhs; \
        return *this; \
    }

    X(+=)
    X(-=)
    X(*=)
    X(/=)
    X(%=)
    X(&=)
    X(|=)
    X(^=)
    X(<<=)
    X(>>=)
#undef X

    // mono operators
#define X(op) \
    template<typename Rhs> \
    auto operator op() const noexcept \
        requires requires(value_type const& a) { op a; } \
    { \
        return op *read(); \
    }

    X(-)
    X(+)
    X(~)
    X(!)
#undef X

    // binary operators
#define X(op) \
    template<typename Rhs> \
    auto operator op(Rhs const& rhs) const noexcept \
        requires requires(value_type const& a, Rhs const& b) { a op b; } \
    { \
        return *read() op rhs; \
    }

    X(==)
    X(<=>)
    X(+)
    X(-)
    X(*)
    X(/)
    X(%)
    X(&)
    X(|)
    X(^)
    X(<<)
    X(>>)
#undef X

    // call operator
    template<typename... Args>
    auto operator()(Args &&... args) const noexcept
        requires requires(value_type const & a, Args &&...args) { a(std::forward<Args>(args)...); }
    {
        return (*read())(std::forward<Args>(args)...);
    }

    // index operator
    // XXX c++23
    template<typename Arg>
    auto operator[](Arg && arg) const noexcept
        requires requires(value_type const & a, Arg &&arg) { a[std::forward<Arg>(arg)]; }
    {
        return (*read())[std::forward<Arg>(arg)];
    }

    // clang-format on

private:
    std::shared_ptr<observable> _observed = {};
    path_type _path = {};
    observable::token_type _observed_cbt = {};
    std::function<void *(void *)> _convert = {};
    notifier_type _notifier;

    /** Construct an observer from an observable.
     */
    observer(
        forward_of<std::shared_ptr<observable>> auto&& observed,
        forward_of<path_type> auto&& path,
        forward_of<void *(void *)> auto&& converter) noexcept :
        _observed(hi_forward(observed)), _path(hi_forward(path)), _convert(hi_forward(converter)), _notifier()
    {
        update_state_callback();
    }

    void read_unlock() const noexcept
    {
        _observed->read_unlock();
    }

    void commit(void const *old_base, void *new_base) const noexcept requires(is_mutable)
    {
        // Only commit and notify when the value has actually changed.
        auto *old_value = convert(old_base);
        auto *new_value = convert(new_base);
        if (*old_value != *new_value) {
            _observed->commit(new_base);
            _observed->notify(old_base, new_base, _path);
        } else {
            _observed->abort(new_base);
        }
        _observed->write_unlock();
    }

    void abort(void *base) const noexcept requires(is_mutable)
    {
        _observed->abort(base);
        _observed->write_unlock();
    }

    value_type *convert(void *base) const noexcept requires(is_mutable)
    {
        return std::launder(static_cast<value_type *>(_convert(base)));
    }

    value_type const *convert(void const *base) const noexcept
    {
        return std::launder(static_cast<value_type const *>(_convert(const_cast<void *>(base))));
    }

    void update_state_callback() noexcept
    {
        _observed_cbt =
            _observed->subscribe(_path, callback_flags::synchronous, [this](void const *old_base, void const *new_base) {
                return _notifier(*convert(old_base), *convert(new_base));
            });
    }

    template<typename>
    friend class observable_value;

    // It is possible to make sub-observables and `observer` to `const_observer`.
    template<typename, bool>
    friend class observer;
};

} // namespace detail

template<typename T>
using observer = detail::observer<T, true>;

template<typename T>
using const_observer = detail::observer<T, false>;

} // namespace hi::inline v1
