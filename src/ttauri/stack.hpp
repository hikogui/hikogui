// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <type_traits>
#include <memory>
#include <initializer_list>
#include "required.hpp"

namespace tt {

template<typename T, size_t MaxSize>
class stack {
public:
    using value_type = T;
    using pointer_type = value_type *;
    using const_pointer_type = value_type const *;
    using reference_type = value_type &;
    using const_reference_type = value_type const &;
    using iterator_type = pointer_type;
    using const_iterator_type = const_pointer_type;
    using size_type = size_t;
    using difference_type = ptrdiff_t;

    stack() noexcept : _top(begin()) {}

    stack(std::initializer_list<value_type> init) noexcept :
        _top(begin())
    {
        for (ttlet &init_item : init) {
            push_back(init_item);            
        }
    }

    [[nodiscard]] iterator_type begin() noexcept
    {
        return std::launder(reinterpret_cast<pointer_type>(&_buffer[0]));
    }

    [[nodiscard]] const_iterator_type begin() const noexcept
    {
        return std::launder(reinterpret_cast<const_pointer_type>(&_buffer[0]));
    }

    [[nodiscard]] const_iterator_type cbegin() const noexcept
    {
        return std::launder(reinterpret_cast<const_pointer_type>(&_buffer[0]));
    }

    [[nodiscard]] iterator_type end() noexcept
    {
        return _top;
    }

    [[nodiscard]] const_iterator_type end() const noexcept
    {
        return _top;
    }

    [[nodiscard]] const_iterator_type cend() const noexcept
    {
        return _top;
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept
    {
        return MaxSize;
    }

    [[nodiscard]] size_type size() const noexcept
    {
        return narrow_cast<size_type>(end() - begin());
    }

    [[nodiscard]] bool full() const noexcept
    {
        return _top == end();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return _top == begin();
    }

    [[nodiscard]] reference_type operator[](size_t index) noexcept
    {
        tt_axiom(index < size());
        return *std::launder(reinterpret_cast<pointer_type>(&_buffer[index]));
    }

    [[nodiscard]] const_reference_type operator[](size_t index) const noexcept
    {
        tt_axiom(index < size());
        return *std::launder(reinterpret_cast<pointer_type>(&_buffer[index]));
    }

    [[nodiscard]] reference_type back() noexcept
    {
        tt_axiom(!empty());
        return *std::launder(reinterpret_cast<pointer_type>(_top - 1));
    }

    [[nodiscard]] const_reference_type back() const noexcept
    {
        tt_axiom(!empty());
        return *std::launder(reinterpret_cast<pointer_type>(_top - 1));
    }

    template<typename... Args>
    [[nodiscard]] void emplace_back(Args &&... args) noexcept
    {
        tt_axiom(!full());
        new (end()) value_type(std::forward<Args>(args)...);
        ++_top;
    }

    template<typename Arg> requires (std::is_convertible_v<Arg,value_type>)
    [[nodiscard]] void push_back(Arg &&arg) noexcept
    {
        emplace_back(std::forward<Arg>(arg));
    }

    void pop_back() noexcept
    {
        tt_axiom(!empty());
        auto *old_item = std::launder(--_top);
        std::destroy_at(old_item);
    }

    /** Pop elements of the stack through the given iterator.
     * Pop elements up to and including the element at new_end.
     */
    void pop_back(iterator_type new_end) noexcept
    {
        while (_top != new_end) {
            pop_back();
        }
    }

    void clear() noexcept
    {
        std::destroy(begin(), end());
        _top = begin();
    }

private:
    std::aligned_storage_t<sizeof(T), std::alignment_of_v<T>> _buffer[MaxSize];
    pointer_type _top;
};

}