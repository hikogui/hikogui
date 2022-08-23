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

template<typename T>
class shared_state_cursor;

namespace detail {

class shared_state_base : public std::enable_shared_from_this<shared_state_base> {
public:
    using notifier_type = notifier<void(void const *, void const *)>;
    using token_type = notifier_type::token_type;
    using function_proto = notifier_type::function_proto;

    constexpr virtual ~shared_state_base() = default;
    shared_state_base(shared_state_base const&) = delete;
    shared_state_base(shared_state_base&&) = delete;
    shared_state_base& operator=(shared_state_base const&) = delete;
    shared_state_base& operator=(shared_state_base&&) = delete;
    constexpr shared_state_base() noexcept = default;

protected:
    using path_type = std::vector<std::string>;

    tree<std::string, notifier_type> _notifiers;

    [[nodiscard]] virtual void const *read() noexcept = 0;
    [[nodiscard]] virtual std::pair<void const *, void *> old_and_copy() noexcept = 0;
    virtual void commit(void const *old_ptr, void *new_ptr, path_type const& path) noexcept = 0;
    virtual void abort(void *new_ptr) noexcept = 0;
    virtual void lock() noexcept = 0;
    virtual void unlock() noexcept = 0;

    [[nodiscard]] token_type
    subscribe(path_type const& path, callback_flags flags, forward_of<function_proto> auto&& function) noexcept
    {
        auto& notifier = _notifiers[path];
        return notifier.subscribe(flags, hi_forward(function));
    }

    void notify(void const *old_ptr, void const *new_ptr, path_type const& path) noexcept
    {
        _notifiers.walk_including_path(path, [old_ptr, new_ptr](notifier_type const& notifier) {
            notifier(old_ptr, new_ptr);
        });
    }

    template<typename O>
    friend class ::hi::shared_state_cursor;
};

/** Shared state of an application.
 *
 * The shared state of an application that can be manipulated by the GUI,
 * preference and other systems.
 *
 * A `shared_cursor` selects a member or indexed element from the shared state,
 * or from another cursor. You can `.read()` or `.copy()` the value pointed to
 * by the cursor to read and manipulate the shared-data.
 *
 * Both `.read()` and `.copy()` take the full shared-state as a whole not allowing
 * other threads to have write access to this reference or copy. A copy will be
 * automatically committed, or may be aborted as well.
 *
 * lifetime:
 * - The lifetime of `shared_cursor` must be within the lifetime of `shared_state`.
 * - The lifetime of `shared_cursor::proxy` must be within the lifetime of `shared_cursor`.
 * - The lifetime of `shared_cursor::const_proxy` must be within the lifetime of `shared_cursor`.
 * - Although `shared_cursor` are created from another `shared_cursor` they internally do not
 *   refer to each other so their lifetime are not connected.
 *
 * @tparam T type used as the shared state.
 */
template<typename T>
class shared_state_impl final : public shared_state_base {
public:
    using value_type = T;

    ~shared_state_impl() = default;

    /** Construct the shared state and initialize the value.
     *
     * @param args The arguments passed to the constructor of the value.
     */
    template<typename... Args>
    constexpr shared_state_impl(Args&&...args) noexcept : _rcu()
    {
        _rcu.emplace(std::forward<Args>(args)...);
    }

    /** Get a cursor to the value.
     *
     * This function returns a cursor to the value object.
     * The cursor is used to start read or write transactions or create other cursors.
     *
     * @return The new cursor pointing to the value object.
     */
    [[nodiscard]] shared_state_cursor<value_type> cursor() const& noexcept
    {
        // clang-format off
        return {
            const_cast<shared_state_impl *>(this)->shared_from_this(),
            std::vector<std::string>{{"/"}},
            [](void *base) -> void * {
                return base;
            }
        };
        // clang-format on
    }

private:
    using path_type = std::vector<std::string>;

    rcu<value_type> _rcu;
    unfair_mutex _write_mutex;

    [[nodiscard]] void const *read() noexcept override
    {
        return _rcu.get();
    }

    [[nodiscard]] std::pair<void const *, void *> old_and_copy() noexcept override
    {
        _write_mutex.lock();
        lock();
        return _rcu.old_and_copy();
    }

    void commit(void const *old_ptr, void *new_ptr, path_type const& path) noexcept override
    {
        _rcu.commit(static_cast<value_type *>(new_ptr));
        notify(old_ptr, new_ptr, path);
        unlock();
        _write_mutex.unlock();
    }

    void abort(void *new_ptr) noexcept override
    {
        _rcu.abort(static_cast<value_type *>(new_ptr));
        unlock();
        _write_mutex.unlock();
    }

    void lock() noexcept override
    {
        _rcu.lock();
    }

    void unlock() noexcept override
    {
        _rcu.unlock();
    }
};

} // namespace detail

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
        _pimpl(std::make_shared<detail::shared_state_impl<value_type>>(std::forward<Args>(args)...))
    {
    }

    /** Get a cursor to the value.
     *
     * This function returns a cursor to the value object.
     * The cursor is used to start read or write transactions or create other cursors.
     *
     * @return The new cursor pointing to the value object.
     */
    [[nodiscard]] shared_state_cursor<value_type> cursor() const noexcept
    {
        return _pimpl->cursor();
    }

    // clang-format off
    /** Get a cursor to a sub-object of value accessed by the index operator.
     *
     * @param index The index used with the index operator of the value.
     * @return The new cursor pointing to a sub-object of the value.
     */
    [[nodiscard]] auto operator[](auto const& index) const& noexcept
        requires requires { cursor()[index]; }
    {
        return cursor()[index];
    }
    // clang-format on

    /** Get a cursor to a member variable of the value.
     *
     * @note This requires the specialization of `hi::selector<value_type>`.
     * @tparam Name the name of the member variable of the value.
     * @return The new cursor pointing to the member variable of the value
     */
    template<basic_fixed_string Name>
    [[nodiscard]] auto get() const& noexcept
    {
        return cursor().get<Name>();
    }

private:
    std::shared_ptr<detail::shared_state_impl<value_type>> _pimpl;
};

template<typename T>
class shared_state_cursor {
public:
    using value_type = T;
    using notifier_type = notifier<void(value_type const&, value_type const&)>;
    using token_type = notifier_type::token_type;
    using function_proto = notifier_type::function_proto;

    /** A proxy object of the shared_state_cursor.
     *
     * The proxy is a RAII object that manages a transaction with the
     * shared-state as a whole, while giving access to only a sub-object
     * of the shared-state.
     */
    class proxy {
    public:
        /** Commits and destruct the proxy object.
         *
         * If `commit()` or `abort()` are called or the proxy object
         * is empty then the destructor does not commit the changes.
         */
        ~proxy() noexcept
        {
            if (_new_base) {
                hi_axiom(_cursor);
                _cursor->commit(_old_base, _new_base);
            }
        }

        proxy(proxy const&) = delete;
        proxy& operator=(proxy const&) = delete;

        proxy(proxy&& other) noexcept :
            _cursor(std::exchange(other._cursor, nullptr)),
            _old_base(std::exchange(other._old_base, nullptr)),
            _new_base(std::exchange(other._new_base, nullptr)),
            _value(std::exchange(other._value, nullptr))
        {
        }

        proxy& operator=(proxy&& other) noexcept
        {
            if (_new_base) {
                hi_axiom(_cursor);
                _cursor->commit(_old_base, _new_base);
            }
            _cursor = std::exchange(other._cursor, nullptr);
            _old_base = std::exchange(other._old_base, nullptr);
            _new_base = std::exchange(other._new_base, nullptr);
            _value = std::exchange(other._value, nullptr);
        }

        /** Construct an empty proxy object.
         */
        constexpr proxy() noexcept = default;

        /** Derefence the value.
         *
         * This function allows reads and modification to the value
         *
         * @note It is undefined behavior to call this function after calling `commit()` or `abort()`
         */
        value_type& operator*() noexcept
        {
            hi_axiom(_new_base);
            hi_axiom(_value);
            return *_value;
        }

        /** Pointer derefence the value.
         *
         * This function allows reads and modification to the value, including
         * calling member functions on the value.
         *
         * @note It is undefined behavior to call this function after calling `commit()` or `abort()`
         */
        value_type *operator->() noexcept
        {
            hi_axiom(_new_base);
            return _value;
        }

        /** Commit the changes to the value early.
         *
         * Calling this function allows to commit earlier than the destructor.
         *
         * @note It is undefined behavior to change the value after commiting.
         */
        void commit() noexcept
        {
            auto tmp_new_base = std::exchange(_new_base, nullptr);
            if (tmp_new_base) {
                hi_axiom(_cursor);
                _cursor->commit(_old_base, tmp_new_base);
            }
        }

        /** Revert any changes to the value.
         *
         * Calling this function allows to abort any changes in the value.
         *
         * @note It is undefined behavior to change the value after aborting.
         */
        void abort() noexcept
        {
            auto tmp_new_base = std::exchange(_new_base, nullptr);
            if (tmp_new_base) {
                hi_axiom(_cursor);
                _cursor->abort(tmp_new_base);
            }
        }

    private:
        shared_state_cursor const *_cursor = nullptr;
        void const *_old_base = nullptr;
        void *_new_base = nullptr;
        value_type *_value = nullptr;

        /** Create a proxy object.
         *
         * @param cursor a pointer to the cursor.
         * @param base a pointer to the dereference rcu-object from the shared_state.
         *             This is needed to commit or abort the shared_state as a whole.
         * @param value a pointer to the sub-object of the shared_state that the cursor
         *              is pointing to.
         */
        proxy(shared_state_cursor const *cursor, void const *old_base, void *new_base, value_type *value) noexcept :
            _cursor(cursor), _old_base(old_base), _new_base(new_base), _value(value)
        {
            hi_axiom(_cursor);
            hi_axiom(_old_base);
            hi_axiom(_new_base);
            hi_axiom(_value);
        }

        friend class shared_state_cursor;
    };

    class const_proxy {
    public:
        ~const_proxy() noexcept
        {
            if (_value) {
                hi_axiom(_cursor);
                _cursor->unlock();
            }
        }

        const_proxy(const_proxy const& other) noexcept : _cursor(other._cursor), _value(other._value)
        {
            if (_value) {
                hi_axiom(_cursor);
                _cursor->lock();
            }
        }

        const_proxy& operator=(const_proxy const& other) noexcept
        {
            if (_value) {
                hi_axiom(_cursor);
                _cursor->unlock();
            }
            _cursor = other._cursor;
            _value = other._value;
            if (_value) {
                hi_axiom(_cursor);
                _cursor->lock();
            }
        }

        const_proxy(const_proxy&& other) noexcept :
            _cursor(std::exchange(other._cursor, nullptr)), _value(std::exchange(other._value, nullptr))
        {
        }

        const_proxy& operator=(const_proxy&& other) noexcept
        {
            if (_value) {
                hi_axiom(_cursor);
                _cursor->unlock();
            }
            _cursor = std::exchange(other._cursor, nullptr);
            _value = std::exchange(other._value, nullptr);
        }

        constexpr const_proxy() noexcept = default;

        [[nodiscard]] value_type const& operator*() const noexcept
        {
            hi_axiom(_cursor);
            hi_axiom(_value);
            return *_value;
        }

        [[nodiscard]] value_type const *operator->() const noexcept
        {
            hi_axiom(_cursor);
            hi_axiom(_value);
            return _value;
        }

    private:
        shared_state_cursor const *_cursor = nullptr;
        value_type const *_value = nullptr;

        const_proxy(shared_state_cursor const *cursor, value_type const *value) noexcept : _cursor(cursor), _value(value) {}

        friend class shared_state_cursor;
    };

    constexpr ~shared_state_cursor() = default;
    constexpr shared_state_cursor(shared_state_cursor const&) noexcept = default;
    constexpr shared_state_cursor(shared_state_cursor&&) noexcept = default;
    constexpr shared_state_cursor& operator=(shared_state_cursor const&) noexcept = default;
    constexpr shared_state_cursor& operator=(shared_state_cursor&&) noexcept = default;

    /** Create a shared_state_cursor linked to an anonymous shared-state.
     */
    constexpr shared_state_cursor(auto&&...args) noexcept :
        _state(std::make_shared<detail::shared_state_impl<value_type>>(hi_forward(args)...)),
        _path{{"/"}},
        _state_cbt(_state->subscribe(_path, callback_flags::synchronous, make_notify_callback())),
        _convert([](void *base) {
            return base;
        })
    {
    }

    const_proxy read() && = delete;
    proxy copy() && = delete;
    const_proxy operator->() && = delete;

    [[nodiscard]] const_proxy read() const& noexcept
    {
        _state->lock();
        return {this, convert(_state->read())};
    }

    value_type operator*() const noexcept
    {
        return *read();
    }

    const_proxy operator->() const& noexcept
    {
        return read();
    }

    [[nodiscard]] proxy copy() const& noexcept
    {
        auto [old_base, new_base] = _state->old_and_copy();
        return {this, old_base, new_base, convert(new_base)};
    }

    shared_state_cursor& operator=(value_type const& rhs) noexcept
    {
        *copy() = rhs;
    }

    shared_state_cursor& operator=(value_type&& rhs) noexcept
    {
        *copy() = std::move(rhs);
    }

    [[nodiscard]] token_type subscribe(callback_flags flags, forward_of<function_proto> auto&& callback) noexcept
    {
        return _notifier.subscribe(flags, hi_forward(callback));
    }

    [[nodiscard]] auto operator[](auto const& index) const noexcept requires(requires() { std::declval<value_type>()[index]; })
    {
        using result_type = std::decay_t<decltype(std::declval<value_type>()[index])>;

        auto new_path = _path;
        new_path.push_back(std::format("[{}]", index));
        return shared_state_cursor<result_type>{
            _state, std::move(new_path), [convert_copy = this->_convert, index](void *base) -> void * {
                return std::addressof((*static_cast<value_type *>(convert_copy(base)))[index]);
            }};
    }

    template<basic_fixed_string Name>
    [[nodiscard]] auto get() const noexcept
    {
        using result_type = std::decay_t<decltype(selector<value_type>{}.get<Name>(std::declval<value_type&>()))>;

        auto new_path = _path;
        new_path.push_back(std::string{Name});
        return shared_state_cursor<result_type>{
            _state, std::move(new_path), [convert_copy = this->_convert](void *base) -> void * {
                return std::addressof(selector<value_type>{}.get<Name>(*static_cast<value_type *>(convert_copy(base))));
            }};
    }

private:
    using path_type = std::vector<std::string>;

    std::shared_ptr<detail::shared_state_base> _state = {};
    path_type _path = {};
    detail::shared_state_base::token_type _state_cbt = {};
    std::function<void *(void *)> _convert = {};
    notifier_type _notifier;

    shared_state_cursor(
        forward_of<std::shared_ptr<detail::shared_state_base>> auto&& state,
        forward_of<path_type> auto&& path,
        forward_of<void *(void *)> auto&& converter) noexcept :
        _state(hi_forward(state)),
        _path(hi_forward(path)),
        _state_cbt(_state->subscribe(_path, callback_flags::synchronous, make_notify_callback())),
        _convert(hi_forward(converter))
    {
    }

    void unlock() const noexcept
    {
        _state->unlock();
    }

    void commit(void const *old_base, void *new_base) const noexcept
    {
        _state->commit(old_base, new_base, _path);
    }

    void abort(void *base) const noexcept
    {
        _state->abort(base);
    }

    value_type *convert(void *base) const noexcept
    {
        return static_cast<value_type *>(_convert(base));
    }

    value_type const *convert(void const *base) const noexcept
    {
        return static_cast<value_type const *>(_convert(const_cast<void *>(base)));
    }

    [[nodiscard]] auto make_notify_callback() const noexcept
    {
        return [this](void const *old_base, void const *new_base) {
            return _notifier(*convert(old_base), *convert(new_base));
        };
    }

    template<typename O>
    friend class detail::shared_state_impl;

    template<typename O>
    friend class shared_state_cursor;
};

} // namespace hi::inline v1
