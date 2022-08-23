// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "shared_state_base.hpp"
#include "../assert.hpp"
#include "../fixed_string.hpp"
#include <cstddef>
#include <functional>
#include <string>
#include <vector>
#include <memory>

namespace hi::inline v1 {

template<typename T>
class shared_state_cursor {
public:
    using value_type = T;
    using token_type = shared_state_base::token_type;
    using function_type = shared_state_base::function_type;

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
            if (_base) {
                hi_axiom(_cursor);
                _cursor->commit(_base);
            }
        }

        proxy(proxy const&) = delete;
        proxy& operator=(proxy const&) = delete;

        proxy(proxy&& other) noexcept :
            _cursor(std::exchange(other._cursor, nullptr)),
            _base(std::exchange(other._base, nullptr)),
            _value(std::exchange(other._value, nullptr))
        {
        }

        proxy& operator=(proxy&& other) noexcept
        {
            if (_base) {
                hi_axiom(_cursor);
                _cursor->commit(_base);
            }
            _cursor = std::exchange(other._cursor, nullptr);
            _base = std::exchange(other._base, nullptr);
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
            hi_axiom(_base);
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
            hi_axiom(_base);
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
            if (auto tmp = std::exchange(_base, nullptr)) {
                hi_axiom(_cursor);
                _cursor->commit(tmp);
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
            if (auto tmp = std::exchange(_base, nullptr)) {
                hi_axiom(_cursor);
                _cursor->abort(tmp);
            }
        }

    private:
        shared_state_cursor const *_cursor = nullptr;
        void *_base = nullptr;
        value_type *_value = nullptr;

        /** Create a proxy object.
         *
         * @param cursor a pointer to the cursor.
         * @param base a pointer to the dereference rcu-object from the shared_state.
         *             This is needed to commit or abort the shared_state as a whole.
         * @param value a pointer to the sub-object of the shared_state that the cursor
         *              is pointing to.
         */
        proxy(shared_state_cursor const *cursor, void *base, value_type *value) noexcept :
            _cursor(cursor), _base(base), _value(value)
        {
            hi_axiom(_cursor);
            hi_axiom(_base);
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

    const_proxy read() && = delete;
    proxy copy() && = delete;

    [[nodiscard]] const_proxy read() const & noexcept
    {
        _state->lock();
        return {this, static_cast<value_type const *>(_convert(const_cast<void *>(_state->read())))};
    }

    [[nodiscard]] proxy copy() const & noexcept
    {
        void *const ptr = _state->copy();
        return {this, ptr, static_cast<value_type *>(_convert(ptr))};
    }

    [[nodiscard]] token_type subscribe(callback_flags flags, function_type callback) noexcept
    {
        return _state->subscribe(_path, flags, callback);
    }

    [[nodiscard]] auto operator[](auto const& index) const noexcept requires(requires() { std::declval<value_type>()[index]; })
    {
        using result_type = std::decay_t<decltype(std::declval<value_type>()[index])>;

        auto new_path = _path;
        new_path.push_back(std::format("[{}]", index));
        return shared_state_cursor<result_type>{
            _state, std::move(new_path), [convert = this->_convert, index](void *base) -> void * {
                return std::addressof((*static_cast<value_type *>(convert(base)))[index]);
            }};
    }

    template<basic_fixed_string Name>
    [[nodiscard]] auto get() const noexcept
    {
        using result_type = std::decay_t<decltype(selector<value_type>{}.get<Name>(std::declval<value_type &>()))>;

        auto new_path = _path;
        new_path.push_back(std::string{Name});
        return shared_state_cursor<result_type>{
            _state, std::move(new_path), [convert=this->_convert](void *base) -> void * {
                return std::addressof(selector<value_type>{}.get<Name>(*static_cast<value_type *>(convert(base))));
            }};
    }

private:
    using path_type = std::vector<std::string>;

    shared_state_base *_state = nullptr;
    path_type _path = {};
    std::function<void *(void *)> _convert = {};

    shared_state_cursor(shared_state_base *state, path_type &&path, std::function<void *(void *)> &&converter) noexcept :
        _state(state), _path(std::move(path)), _convert(std::move(converter))
    {
    }

    void unlock() const noexcept
    {
        _state->unlock();
    }

    void commit(void *base) const noexcept
    {
        _state->commit(base, _path);
    }

    void abort(void *base) const noexcept
    {
        _state->abort(base);
    }

    template<typename O>
    friend class shared_state;

    template<typename O>
    friend class shared_state_cursor;
};

} // namespace hi::inline v1
