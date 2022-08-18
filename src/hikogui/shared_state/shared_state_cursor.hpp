
#pragma once

#include "shared_state_path.hpp"
#include "shared_state.hpp"
#include "../assert.hpp"
#include "../memory.hpp"
#include <cstddef>

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
            if (_rcu_ptr) {
                _state->commit(_rcu_ptr, _path.path());
            }
        }

        proxy(proxy const&) = delete;
        proxy& operator=(proxy const&) = delete;

        proxy(proxy&& other) noexcept :
            _state(std::exchange(other._state, nullptr)),
            _path(std::exchange(other._path, nullptr)),
            _rcu_ptr(std::exchange(other._rcu_ptr, nullptr)),
            _value(std::exchange(other._value, nullptr))
        {
        }

        proxy& operator=(proxy&& other) noexcept
        {
            if (_rcu_ptr) {
                _state->commit(rcu_ptr, _path.path());
            }
            _state = std::exchange(other._state, nullptr);
            _path = std::exchange(other._path, nullptr);
            _rcu_ptr = std::exchange(other._rcu_ptr, nullptr);
            _value = std::exchange(other._value, nullptr);
        }

        constexpr proxy() noexcept = default;

        proxy(shared_state_base *state, std::shared_ptr<shared_state_path> path, void *rcu_ptr, value_type *value) noexcept :
            _state(state), _path(std::move(path)), _rcu_ptr(rcu_ptr), _value(value)
        {
            hi_axiom(_state);
            hi_axiom(_path);
            hi_axiom(_rcu_ptr);
            hi_axiom(_value);
        }

        value_type& operator*() noexcept
        {
            hi_axiom(_rcu_ptr);
            hi_axiom(_value);
            return *_value;
        }

        value_type *operator->() noexcept
        {
            hi_axiom(_rcu_ptr);
            return _value;
        }

        void commit() noexcept
        {
            if (auto tmp = std::exchange(_rcu_ptr, nullptr)) {
                _state->commit(_rcu_ptr, _path.path());
            }
        }

        void abort() noexcept
        {
            if (auto tmp = std::exchange(_rcu_ptr, nullptr)) {
                _state->abort(_rcu_ptr);
            }
        }

    private:
        shared_state_base *_state = nullptr;
        shared_ptr<shared_state_path> _path = nullptr;
        void *_rcu_ptr = nullptr;
        value_type *_value = nullptr;
    };

    class const_proxy {
    public:
        ~const_proxy() noexcept
        {
            if (_state) {
                _state->unlock();
            }
        }

        const_proxy(const_proxy const& other) noexcept : _state(other._state), _value(other._value)
        {
            if (_state) {
                _state->lock();
            }
        }

        const_proxy& operator=(const_proxy const& other) noexcept
        {
            if (_state) {
                _state->unlock();
            }
            _state = other._state;
            _value = other._value;
            if (_state) {
                _state->lock();
            }
        }

        const_proxy(const_proxy&& other) noexcept :
            _state(std::exchange(other._state, nullptr)), _value(std::exchange(other._value, nullptr))
        {
        }

        const_proxy& operator=(const_proxy&& other) noexcept
        {
            if (_state) {
                _state->unlock();
            }
            _state = std::exchange(other._state, nullptr);
            _value = std::exchange(other._value, nullptr);
        }

        constexpr const_proxy() noexcept = default;
        const_proxy(shared_state_base *state, value_type const *value) noexcept : _state(state), _value(value) {}

        [[nodiscard]] value_type const& operator*() const noexcept
        {
            hi_axiom(_rcu_ptr);
            return *_value;
        }

        [[nodiscard]] value_type const *operator->() const noexcept
        {
            return _value;
        }

    private:
        shared_state_base *_state = nullptr;
        value_type const *_value = nullptr;
    };

    shared_state_cursor(shared_state_base *state, std::shared_ptr<shared_state_path> path) noexcept :
        _state(state), _path(std::move(path))
    {
    }

    [[nodiscard]] const_proxy read() const noexcept
    {
        _state->lock();
        return {_state, static_cast<value_type *>(_path->get(_state->read()))};
    }

    [[nodiscard]] proxy copy() const noexcept
    {
        void *const ptr = _state->copy();
        return {_state, _path, ptr, static_cast<value_type *>(_path->get(ptr))};
    }

    [[nodiscard]] token_type subscribe(callback_flags flags, function_type callback) noexcept
    {
        return _state->subscribe(_path->path(), flags, callback);
    }

    template<basic_fixed_string Name>
    [[nodiscard]] auto by_name() const noexcept
    {
        using output_type = decltype(selector<value_type>{}.get<Name>(std::declval<value_type>()));
        return shared_state_cursor<output_type>{_state, _path.by_name<value_type, Name>()};
    }

    [[nodiscard]] auto by_index(auto const &index) const noexcept
    {
        using output_type = decltype(std::declval<value_tpe>{}[index]);
        return shared_state_cursor<output_type>{_state, _path.by_index<value_type>(index)};
    }

    [[nodiscard]] auto by_index(auto const &index) const noexcept
    {
        using output_type = decltype(std::declval<value_tpe>{}[index]);
        return shared_state_cursor<output_type>{_state, _path.by_index<value_type>(index)};
    }

    [[nodiscard]] auto operator[](auto const& index) noexcept requires(requires() { std::declval<value_tpe>{}[index]; })
    {
        return by_index(index);
    }

private:
    shared_state_base *_state = nullptr;
    shared_state_path _path = {};
};

} // namespace hi::inline v1
