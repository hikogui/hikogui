// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "notifier.hpp"
#include <memory>

namespace hi::inline v1 {

template<typename T>
class cursor;

template<typename T>
class st_proxy {
public:
    using value_type = T;

    ~st_proxy();
    void commit();

    value_type &operator*() const noexcept
    {
        hi_axiom(_value_ptr != nullptr);
        return *_value_ptr;
    }

    value_type *operator->() const noexcept
    {
        return _value_ptr;
    }

private:
    bool _committed = false;
    value_type *_value_ptr = nullptr;
    cursor<T> *_cursor_ptr = nullptr;
};

template<typename T>
class st_const_proxy {
public:
    using value_type = const T;

    ~st_const_proxy();

private:
    value_type *_value_ptr = nullptr;

};

class st_awaiter {

};

template<typename T>
class st_cursor {
public:
    using value_type = T;
    using notifier_type = notifier<>;
    using token_type = notifier_type::token_type;

    st_const_proxy<value_type> read();
    st_proxy<value_type> write();

    template<typename Func>
    token_type subscribe(Func&& func);

    st_awaiter operator co_await();

private:
    notifier_type _notifier = {};
};

template<typename T>
void st_proxy<T>::~st_proxy() noexcept
{
    if (not _committed) {
        hi_axiom(_cursor_ptr != nullptr);
        _cursor_ptr->abort();
    }
}

template<typename T>
void st_proxy<T>::commit() noexcept
{
    hi_axiom(not _committed);
    hi_axiom(_cursor_ptr != nullptr);
    _cursor_ptr->commit();
    _commitded = true;
}

class st_cursor_expression {

};

template<typename T>
class shared_state;

template<typename T>
class shared_proxy {
public:
    using value_type = T;

    constexpr shared_proxy() noexcept = default;
    constexpr shared_proxy(shared_state &state, value_type &value) noexcept : _state(&state), _value(&value) {}

    ~shared_proxy()
    {
        if (_value) {
            _state->write(_value);
        }
    }

    void abort() noexcept
    {
        if (auto old_value = std::exchange(_value, nullptr)) {
            _state->abort(old_value);
        }
    }

private:
    shared_state<T> *_state = nullptr;
    value_type *_value = nullptr;

};

template<typename T>
class const_shared_proxy {
public:
    using value_type = const T;

    constexpr const_shared_proxy() noexcept = default;
    constexpr const_shared_proxy(shared_state &state, value_type &value) noexcept : _state(&state), _value(&value) {}

    ~const_shared_proxy()
    {
        if (_value) {
            _state->unlock();
        }
    }

private:
    shared_state<T> *_state = nullptr;
    value_type *_value = nullptr;
}

template<typename T>
class shared_cursor {
public:
    using value_type = T; 

    [[nodiscard]] const_proxy read() const noexcept;

    [[nodiscard]] proxy copy() const noexcept;

    template<typename NewType>
    shared_cursor<NewType> make_cursor(std::function<NewType &(T &)> rhs) const noexcept;

    token_type subscribe(std::function<void()> func);
};

template<typename T>
class shared_state {
public:
    using value_type = T;

    constexpr shared_state(allocator_type allocator = allocator_type{}) noexcept : _allocator(allocator) {}
    ~shared_state() = default;
    shared_state(shared_state const&) = delete;
    shared_state(shared_state&&) = delete;
    shared_state &operator=(shared_state const&) = delete;
    shared_state &operator=(shared_state&&) = delete;

    value_type const *start_read();
    void finish_read(value_type const *ptr);

    value_type *start_write();
    void commit_write(value_type *ptr);
    void abort_write(value_type *ptr);

    st_cursor make_cursor(st_cursor_expression expression);

    void lock() noexcept
    {
        _rcu.lock();
    }

    void unlock() noexcept
    {
        _rcu.unlock();
    }

    const_shared_proxy<value_type> get() noexcept
    {
        lock();
        return {*this, *_rcu->get()};
    }

    shared_proxy<value_type> copy() noexcept
    {
        lock();
        return {*this, *_rcu->copy()};
    }

private:
    rcu<value_type> _rcu;
};














}
