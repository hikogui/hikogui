// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "concepts.hpp"
#include <vector>
#include <cstddef>

namespace hi::inline v1 {

template<typename T>
class undo_stack {
public:
    using value_type = T;

    constexpr undo_stack(undo_stack const &) noexcept = default;
    constexpr undo_stack(undo_stack &&) noexcept = default;
    constexpr undo_stack &operator=(undo_stack const &) noexcept = default;
    constexpr undo_stack &operator=(undo_stack &&) noexcept = default;
    constexpr undo_stack(size_t max_depth) noexcept : _stack{}, _max_depth(max_depth), _cursor(0) {}

    template<typename... Args>
    constexpr void emplace(Args &&... args) noexcept
    {
        push(value_type{std::forward<Args>(args)...});
    }

    [[nodiscard]] constexpr bool can_undo() const noexcept
    {
        return _cursor != 0;
    }

    template<typename... Args>
    [[nodiscard]] constexpr value_type const &undo(Args &&...args) noexcept
    {
        hi_assert(can_undo());
        if (_first_undo) {
            // On the first undo, we need to emplace the last state. So that redo
            // will be able to get back to state before the undo.
            push(value_type{std::forward<Args>(args)...});
            // The state we just added should be skipped.
            --_cursor;
            _first_undo = false;
        }
        return _stack[--_cursor];
    }

    [[nodiscard]] constexpr bool can_redo() const noexcept
    {
        return (_cursor + 1) < _stack.size();
    }

    [[nodiscard]] constexpr value_type const &redo() const noexcept
    {
        hi_assert(can_redo());
        return _stack[++_cursor];
    }

private:
    std::vector<value_type> _stack;
    size_t _max_depth;
    mutable size_t _cursor;
    mutable bool _first_undo = true;

    template<forward_of<value_type> Value>
    constexpr void push(Value &&value) noexcept
    {
        hi_assert(_cursor <= _stack.size());
        _stack.erase(_stack.begin() + _cursor, _stack.end());

        if (_stack.size() > _max_depth) {
            _stack.erase(_stack.begin());
        }

        _stack.push_back(std::forward<Value>(value));
        _cursor = _stack.size();

        _first_undo = true;
    }
};


}
