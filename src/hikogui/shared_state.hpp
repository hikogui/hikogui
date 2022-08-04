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

template<typename T, typename Allocator = std::allocator<T>>
class shared_state {
public:
    using value_type = T;
    using allocator_type = Allocator;

    ~shared_state() {}
    shared_state(shared_state const&) = delete;
    shared_state(shared_state&&) = delete;
    shared_state &operator=(shared_state const&) = delete;
    shared_state &operator=(shared_state&&) = delete;
    constexpr shared_state(allocator_type allocator = allocator_type{}) noexcept : _allocator(allocator) {}

    value_type const *start_read();
    void finish_read(value_type const *ptr);

    value_type *start_write();
    void commit_write(value_type *ptr);
    void abort_write(value_type *ptr);

    st_cursor make_cursor(st_cursor_expression expression);

private:
    allocator_type _allocator;
};














}