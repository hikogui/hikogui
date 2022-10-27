// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>
#include <memory>
#include <initializer_list>
#include "utility.hpp"
#include "cast.hpp"

namespace hi::inline v1 {

/** A static sized stack.
 * This stack is designed around the functionality of a std::vector, except the
 * data is allocated locally inside the object instead of on the heap.
 *
 * Because the stack can not grow or shrink, the iterators remain valid over the
 * lifetime of the stack.
 */
template<typename T, std::size_t MaxSize>
class stack {
public:
    using value_type = T;
    using pointer_type = value_type *;
    using const_pointer_type = value_type const *;
    using reference_type = value_type &;
    using const_reference_type = value_type const &;
    using iterator_type = pointer_type;
    using const_iterator_type = const_pointer_type;
    using size_type = std::size_t;
    using difference_type = ptrdiff_t;

    /** Construct an empty stack.
     */
    stack() noexcept : _top(begin()) {}

    /** Construct a stack with the given data.
     * @param init An initializer_list of items to add to the stack.
     */
    stack(std::initializer_list<value_type> init) noexcept : _top(begin())
    {
        for (hilet &init_item : init) {
            push_back(init_item);
        }
    }

    ~stack() noexcept
    {
        clear();
    }

    /** Get an iterator to the first element on the stack.
     * @return An iterator to the first element on the stack.
     */
    [[nodiscard]] iterator_type begin() noexcept
    {
        return std::launder(reinterpret_cast<pointer_type>(&_buffer[0]));
    }

    /** Get an iterator to the first element on the stack.
     * @return An iterator to the first element on the stack.
     */
    [[nodiscard]] const_iterator_type begin() const noexcept
    {
        return std::launder(reinterpret_cast<const_pointer_type>(&_buffer[0]));
    }

    /** Get an iterator to the first element on the stack.
     * @return An iterator to the first element on the stack.
     */
    [[nodiscard]] const_iterator_type cbegin() const noexcept
    {
        return std::launder(reinterpret_cast<const_pointer_type>(&_buffer[0]));
    }

    /** Get an iterator to the last element on the stack.
     * @return An iterator one beyond the last element on the stack.
     */
    [[nodiscard]] iterator_type end() noexcept
    {
        return _top;
    }

    /** Get an iterator to the last element on the stack.
     * @return An iterator one beyond the last element on the stack.
     */
    [[nodiscard]] const_iterator_type end() const noexcept
    {
        return _top;
    }

    /** Get an iterator to the last element on the stack.
     * @return An iterator one beyond the last element on the stack.
     */
    [[nodiscard]] const_iterator_type cend() const noexcept
    {
        return _top;
    }

    /** The maximum number of elements that fit on the stack.
     * @return the maximum number of elements that fit on the stack.
     */
    [[nodiscard]] constexpr size_type max_size() const noexcept
    {
        return MaxSize;
    }

    /** The number of elements that fit on the stack.
     * @return the number of elements that fit on the stack.
     */
    [[nodiscard]] size_type size() const noexcept
    {
        return narrow_cast<size_type>(end() - begin());
    }

    /** Check if the stack is full.
     * @return True if the stack is full, empty if otherwise.
     */
    [[nodiscard]] bool full() const noexcept
    {
        return _top == (begin() + max_size());
    }

    /** Check if the stack is empty.
     * @return True if the stack is empty, empty if otherwise.
     */
    [[nodiscard]] bool empty() const noexcept
    {
        return _top == begin();
    }

    /** Get a reference to an element on the stack at an index.
     * No bounds checking is performed.
     * @return True if the stack is empty, empty if otherwise.
     */
    [[nodiscard]] reference_type operator[](std::size_t index) noexcept
    {
        hi_assert_bounds(index, *this);
        return *std::launder(reinterpret_cast<pointer_type>(&_buffer[index]));
    }

    /** Get a reference to an element on the stack at an index.
     * No bounds checking is performed.
     * @return True if the stack is empty, empty if otherwise.
     */
    [[nodiscard]] const_reference_type operator[](std::size_t index) const noexcept
    {
        hi_assert_bounds(index, *this);
        return *std::launder(reinterpret_cast<pointer_type>(&_buffer[index]));
    }

    /** Get a reference to an element on the stack at an index.
     * @throws std::out_of_range when the index points beyond the top of the stack.
     * @return True if the stack is empty, empty if otherwise.
     */
    [[nodiscard]] reference_type at(std::size_t index) noexcept
    {
        if (index >= size()) {
            throw std::out_of_range("stack::at");
        }
        return (*this)[index];
    }

    /** Get a reference to an element on the stack at an index.
     * @throws std::out_of_range when the index points beyond the top of the stack.
     * @return True if the stack is empty, empty if otherwise.
     */
    [[nodiscard]] const_reference_type at(std::size_t index) const noexcept
    {
        if (index >= size()) {
            throw std::out_of_range("stack::at");
        }
        return (*this)[index];
    }

    /** Get a reference to the element at the top of the stack.
     * Calling `back()` on an empty container causes undefined behavior.
     */
    [[nodiscard]] reference_type back() noexcept
    {
        hi_axiom(!empty());
        return *std::launder(reinterpret_cast<pointer_type>(_top - 1));
    }

    /** Get a reference to the element at the top of the stack.
     * Calling `back()` on an empty container causes undefined behavior.
     */
    [[nodiscard]] const_reference_type back() const noexcept
    {
        hi_axiom(!empty());
        return *std::launder(reinterpret_cast<pointer_type>(_top - 1));
    }

    /** Construct an object after the current top of the stack.
     * @tparam Args The types of each argument
     * @param args The arguments for the constructor of `value_type`.
     */
    template<typename... Args>
    void emplace_back(Args &&...args) noexcept
    {
        hi_axiom(!full());
        new (end()) value_type(std::forward<Args>(args)...);
        ++_top;
    }

    /** Push a new value to after the current top of the stack.
     * @tparam Arg The type of an object that can be converted to `value_type`
     * @param arg The object to be pushed on the stack.
     */
    template<typename Arg>
    requires(std::is_convertible_v<Arg, value_type>) void push_back(Arg &&arg) noexcept
    {
        emplace_back(std::forward<Arg>(arg));
    }

    /** Remove the value at the top of the stack.
     * It is undefined behaviour to call this function on an empty stack.
     */
    void pop_back() noexcept
    {
        hi_axiom(!empty());
        auto *old_item = std::launder(--_top);
        std::destroy_at(old_item);
    }

    /** Pop elements of the stack through the given iterator.
     * Pop elements up to and including the element at new_end.
     * @param new_end Iterator to the object to be removed.
     */
    void pop_back(iterator_type new_end) noexcept
    {
        while (_top != new_end) {
            pop_back();
        }
    }

    /** Remove all elements from the stack.
     */
    void clear() noexcept
    {
        std::destroy(begin(), end());
        _top = begin();
    }

private:
    std::aligned_storage_t<sizeof(T), std::alignment_of_v<T>> _buffer[MaxSize];
    pointer_type _top;
};

} // namespace hi::inline v1
