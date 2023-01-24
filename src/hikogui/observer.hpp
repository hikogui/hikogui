// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "observable_value.hpp"
#include "observable.hpp"
#include "utility/module.hpp"

namespace hi::inline v1 {

/** A observer pointing to the whole or part of a observable.
 *
 * A observer will point to a observable that was created, or possibly
 * an anonymous observable, which is created when a observer is created
 * as empty.
 *
 * @tparam T The type of observer.
 */
template<typename T>
class observer {
public:
    using value_type = T;
    using notifier_type = notifier<void(value_type)>;
    using callback_token = notifier_type::callback_token;
    using callback_proto = notifier_type::callback_proto;
    using awaiter_type = notifier_type::awaiter_type;
    using path_type = observable_msg::path_type;

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

        _proxy(_proxy const& other) noexcept
            requires(not is_writing)
            : _observer(other._observer), _base(other._base), _value(other._value)
        {
            _observer->read_lock();
        }

        _proxy& operator=(_proxy const& other) noexcept
            requires(not is_writing)
        {
            _commit();
            _observer = other._observer;
            _base = other._base;
            _value = other._value;
            _observer->read_lock();
            return *this;
        };

        _proxy(_proxy&& other) noexcept :
            _observer(std::exchange(other._observer, nullptr)),
            _base(std::exchange(other._base, nullptr)),
            _value(std::exchange(other._value, nullptr))
        {
        }

        _proxy& operator=(_proxy&& other) noexcept
        {
            _commit();
            _observer = std::exchange(other._observer, nullptr);
            _base = std::exchange(other._base, nullptr);
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
            hi_assert_not_null(_value);
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
            hi_assert_not_null(_value);
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
            hi_assert_not_null(_value);
            return _value;
        }

        /** Pointer dereference the value.
         *
         * This function allows reads and modification to the value, including
         * calling member functions on the value.
         *
         * @note It is undefined behavior to call this function after calling `commit()` or `abort()`
         */
        pointer operator&() noexcept
        {
            hi_assert_not_null(_value);
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
            hi_assert_not_null(_value);
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
        decltype(auto) operator()(Args &&... args) noexcept
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
        decltype(auto) operator[](Arg && arg) noexcept
            requires is_writing and requires(value_type & a, Arg &&arg) { a[std::forward<Arg>(arg)]; }
        {
            return (*_value)[std::forward<Arg>(arg)];
        }

        // clang-format on

    private:
        observer const *_observer = nullptr;
        void_pointer _base = nullptr;
        pointer _value = nullptr;

        /** Create a proxy object.
         *
         * @param observer a pointer to the observer.
         * @param base a pointer to the dereference rcu-object from the shared_state.
         *             This is needed to commit or abort the shared_state as a whole.
         * @param value a pointer to the sub-object of the shared_state that the observer
         *              is pointing to.
         */
        _proxy(observer const *observer, void_pointer base, pointer value) noexcept :
            _observer(observer), _base(base), _value(value)
        {
            hi_assert_not_null(_observer);
            hi_assert_not_null(_base);
            hi_assert_not_null(_value);
        }

        void _commit() noexcept
        {
            if (_observer != nullptr) {
                if constexpr (is_writing) {
                    _observer->commit(_base);
                } else {
                    _observer->read_unlock();
                }
            }
        }

        void _abort() noexcept
        {
            if (_observer != nullptr) {
                if constexpr (is_writing) {
                    _observer->abort(_base);
                } else {
                    _observer->read_unlock();
                }
            }
        }

        friend class observer;
    };

    using proxy = _proxy<true>;
    using const_proxy = _proxy<false>;

    constexpr ~observer() = default;

    // static_assert(is_forward_of_v<std::shared_ptr<observable>, group_ptr<observable,observable_msg>>);

    /** Create an observer from an observable.
     *
     * @param observed The `observable` which will be observed by this observer.
     */
    observer(forward_of<std::shared_ptr<observable>> auto&& observed) noexcept :
        observer(group_ptr<observable>{hi_forward(observed)}, path_type{}, [](void *base) {
            return base;
        })
    {
    }

    /** Create a observer linked to an anonymous default initialized observed-value.
     *
     * @note marked 'explicit' so that accidental assignment with {} is not allowed.
     */
    constexpr explicit observer() noexcept : observer(std::make_shared<observable_value<value_type>>()) {}

    /** Create a observer linked to an anonymous observed-value.
     */
    constexpr observer(std::convertible_to<value_type> auto&& value) noexcept :
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

    /** Copy assign.
     *
     * @note callback subscriptions remain unchanged and are not copied.
     * @param other The other observer.
     * @return this
     */
    constexpr observer& operator=(observer const& other) noexcept
    {
        _observed = other._observed;
        _path = other._path;
        _convert = other._convert;

        // Rewire the callback subscriptions and notify listeners to this observer.
        update_state_callback();
        _observed->read_lock();
        _observed->notify_group_ptr(observable_msg{_observed->read(), _path});
        _observed->read_unlock();
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
        _observed = std::move(other._observed);
        _path = std::move(other._path);
        _convert = std::move(other._convert);
        other.reset();

        // Rewire the callback subscriptions and notify listeners to this observer.
        update_state_callback();
        _observed->read_lock();
        _observed->notify_group_ptr({_observed->read(), _path});
        _observed->read_unlock();
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
        void const *base = _observed->read();
        return const_proxy{this, base, convert(base)};
    }
    const_proxy read() && = delete;

    /** Make a copy of the observed value for modification.
     *
     * @return A proxy object used to modify the data being observed.
     */
    [[nodiscard]] proxy copy() const& noexcept
    {
        _observed->write_lock();
        void const *old_base = _observed->read();
        void *new_base = _observed->copy(old_base);
        return proxy(this, new_base, convert(new_base));
    }

    /** Subscribe a callback to this observer.
     *
     * @param flags The callback flags on how to call the function.
     * @param function The function used as callback in the form `void(value_type const &old_value, value_type const &new_value)`
     * @return A callback-token used to extend the lifetime of the callback function.
     */
    [[nodiscard]] callback_token
    subscribe(forward_of<callback_proto> auto&& function, callback_flags flags = callback_flags::synchronous) noexcept
    {
        return _notifier.subscribe(hi_forward(function), flags);
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
    [[nodiscard]] auto get(auto const& index) const noexcept
        requires(requires() { std::declval<value_type>()[index]; })
    {
        using result_type = std::decay_t<decltype(std::declval<value_type>()[index])>;

        auto new_path = _path;
        new_path.push_back(std::format("[{}]", index));
        return observer<result_type>{
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
    template<fixed_string Name>
    [[nodiscard]] auto get() const noexcept
    {
        using result_type = std::decay_t<decltype(selector<value_type>{}.get<Name>(std::declval<value_type&>()))>;

        auto new_path = _path;
        new_path.push_back(std::string{Name});
        // clang-format off
        return observer<result_type>(
            _observed,
            std::move(new_path),
            [convert_copy = this->_convert](void *base) -> void * {
                return std::addressof(selector<value_type>{}.get<Name>(
                    *std::launder(static_cast<value_type *>(convert_copy(base)))));
            });
        // clang-format on
    }

    //
    // Convenient functions / operators working on temporary proxies.
    //

    // clang-format off

    /** Assign a new value to the observed value.
     */
    template<typename Rhs>
    observer& operator=(Rhs&& rhs) noexcept
        requires requires (value_type &a, Rhs &&b) { a = std::forward<Rhs>(b); }
    {
        *copy() = std::forward<Rhs>(rhs);
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

    // prefix operators
#define X(op) \
    template<typename Rhs> \
    observer& operator op() noexcept \
        requires requires(value_type& a) { op a; } \
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
        requires requires(value_type& a) { a op; } \
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
        requires requires(value_type& a, Rhs const& b) { a op b; } \
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
    using observed_type = group_ptr<observable>;
    observed_type _observed = {};
    path_type _path = {};
    std::function<void *(void *)> _convert = {};
    notifier_type _notifier;
#ifndef NDEBUG
    value_type _debug_value;
#endif

    /** Construct an observer from an observable.
     */
    observer(
        forward_of<observed_type> auto&& observed,
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

    void commit(void *base) const noexcept
    {
        if constexpr (requires(value_type const& a, value_type const& b) { a == b; }) {
            // Only commit and notify when the value has actually changed.
            // Since there is a write-lock being held, _observed->read() will be the previous value.
            if (*convert(_observed->read()) != *convert(base)) {
                _observed->commit(base);
                _observed->notify_group_ptr({base, _path});
            } else {
                _observed->abort(base);
            }
        } else {
            _observed->commit(base);
            _observed->notify_group_ptr({base, _path});
        }
        _observed->write_unlock();
    }

    void abort(void *base) const noexcept
    {
        _observed->abort(base);
        _observed->write_unlock();
    }

    value_type *convert(void *base) const noexcept
    {
        return std::launder(static_cast<value_type *>(_convert(base)));
    }

    value_type const *convert(void const *base) const noexcept
    {
        return std::launder(static_cast<value_type const *>(_convert(const_cast<void *>(base))));
    }

    void update_state_callback() noexcept
    {
        _observed.subscribe([this](observable_msg const& msg) {
            hilet [msg_it, this_it] = std::mismatch(msg.path.cbegin(), msg.path.cend(), _path.cbegin(), _path.cend());
            // If the message's path is fully within the this' path, then this is a sub-path.
            // If this' path is fully within the message's path, then this is along the path.
            if (msg_it == msg.path.cend() or this_it == _path.cend()) {
#ifndef NDEBUG
                _notifier(_debug_value = *convert(msg.ptr));
#else
                _notifier(*convert(msg.ptr));
#endif
            }
        });

#ifndef NDEBUG
        _observed->read_lock();
        _debug_value = *convert(_observed->read());
        _observed->read_unlock();
#endif
    }

    // It is possible to make sub-observables.
    template<typename>
    friend class observer;
};

/** A type-trait for observer arguments.
 *
 * returns `type` with the following forms of @a T:
 *  - type
 *  - type &
 *  - type const &
 *  - type &&
 *  - observer<type>
 *  - observer<type> &
 *  - observer<type> const &
 *  - observer<type> &&
 */
template<typename T>
struct observer_decay {
    using type = std::decay_t<T>;
};

// clang-format off
template<typename T> struct observer_decay<observer<T>> { using type = T; };
template<typename T> struct observer_decay<observer<T> &> { using type = T; };
template<typename T> struct observer_decay<observer<T> const &> { using type = T; };
template<typename T> struct observer_decay<observer<T> &&> { using type = T; };

// clang-format on

template<typename T>
using observer_decay_t = observer_decay<T>::type;

template<typename Context, typename Expected>
struct is_forward_of<Context, observer<Expected>> :
    std::conditional_t<std::is_convertible_v<Context, observer<Expected>>, std::true_type, std::false_type> {};

} // namespace hi::inline v1
