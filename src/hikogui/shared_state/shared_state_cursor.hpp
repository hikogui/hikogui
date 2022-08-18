// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "shared_state_base.hpp"
#include "../assert.hpp"
#include "../memory.hpp"
#include "../fixed_string.hpp"
#include <cstddef>
#include <functional>
#include <string>

namespace hi::inline v1 {

template<typename T>
class shared_state_cursor {
public:
    using value_type = T;
    using token_type = shared_state_base::token_type;
    using function_type = shared_state_base::function_type;

    class proxy {
    public:
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

        constexpr proxy() noexcept = default;

        proxy(shared_state_cursor const *cursor, void *base, value_type *value) noexcept :
            _cursor(cursor), _base(base), _value(value)
        {
            hi_axiom(_cursor);
            hi_axiom(_base);
            hi_axiom(_value);
        }

        value_type& operator*() noexcept
        {
            hi_axiom(_base);
            hi_axiom(_value);
            return *_value;
        }

        value_type *operator->() noexcept
        {
            hi_axiom(_base);
            return _value;
        }

        void commit() noexcept
        {
            if (auto tmp = std::exchange(_base, nullptr)) {
                hi_axiom(_cursor);
                _cursor->commit(tmp);
            }
        }

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
        const_proxy(shared_state_cursor const *cursor, value_type const *value) noexcept : _cursor(cursor), _value(value) {}

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
    };

    shared_state_cursor(shared_state_base *state, std::string &&path, std::function<void *(void *)> &&converter) noexcept :
        _state(state), _path(std::move(path)), _convert(std::move(converter))
    {
    }

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

        return shared_state_cursor<result_type>{
            _state, std::format("{}[{}]", _path, index), [convert = this->_convert, index](void *base) -> void * {
                return std::addressof((*static_cast<value_type *>(convert(base)))[index]);
            }};
    }

    template<basic_fixed_string Name>
    [[nodiscard]] auto _() const noexcept
    {
        using result_type = std::decay_t<decltype(selector<value_type>{}.get<Name>(std::declval<value_type &>()))>;

        return shared_state_cursor<result_type>{
            _state, std::format("{}.{}", _path, Name), [convert=this->_convert](void *base) -> void * {
                return std::addressof(selector<value_type>{}.get<Name>(*static_cast<value_type *>(convert(base))));
            }};
    }

private:
    shared_state_base *_state = nullptr;
    std::string _path = {};
    std::function<void *(void *)> _convert = {};

    void unlock() const noexcept
    {
        _state->unlock();
    }

    void commit(void *base) const noexcept
    {
        _state->commit(base, _path);
    }

};

} // namespace hi::inline v1
