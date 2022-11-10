// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file lean_vector.hpp Defined lean_vector<>.
 */

#pragma once

#include "cast.hpp"
#include <bit>
#include <new>
#include <cstddef>
#include <memory>
#include <algorithm>
#include <iterator>

namespace hi { inline namespace v1 {

/** Lean-vector with (SVO) short-vector-optimization.
 *
 * The maximum number of items in SVO are:`(sizeof(T *) * 3 - 1) / sizeof(T)`
 */
template<typename T>
class lean_vector {
public:
    using value_type = T;
    using pointer = value_type *;
    using const_pointer = value_type const *;
    using reference = value_type&;
    using const_reference = value_type const&;
    using iterator = value_type *;
    using const_iterator = value_type const *;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using allocator_type = std::allocator<value_type>;

    /** The maximum number of items that can fit without allocation.
     */
    static constexpr size_t short_capacity = (sizeof(pointer) * 3 - 1) / sizeof(value_type);

    /** The allocator_type used to allocate items.
     */
    constexpr allocator_type get_allocator() const noexcept
    {
        static_assert(std::allocator_traits<allocator_type>::is_always_equal::value);
        return allocator_type{};
    }

    /** Construct an empty vector.
     */
    constexpr lean_vector() noexcept = default;

    /** Destruct the vector.
     */
    ~lean_vector()
    {
        clear();
        shrink_to_fit();
    }

    /** Copy-construct a vector.
     *
     * This will copy a vector. If the copy requires an allocation
     * the new allocation will fit the number of items exactly.
     *
     * @param other The vector to copy.
     */
    lean_vector(lean_vector const& other)
    {
        hilet other_size = other.size();
        hilet is_short = _reserve<false>(other_size);
        std::uninitialized_copy_n(other.begin(), other_size, begin());
        _set_size(other_size, is_short);
    }

    /** Move-construct a vector.
     *
     * This will steal the allocation from the other vector, or move
     * the items if the number of items is less than or equal to `short_capacity`.
     *
     * @param other The vector to move.
     */
    lean_vector(lean_vector&& other) noexcept
    {
        if (other._is_short()) {
            hilet other_size = other._short_size();
            std::uninitialized_move_n(other.begin(), other_size, begin());
            other._set_short_size(0);
            _set_short_size(other_size);

        } else {
            _ptr = std::exchange(other._ptr, nullptr);
            _end = std::exchange(other._end, nullptr);
            _cap = std::exchange(other._cap, nullptr);
        }
    }

    /** Copy-assign a vector.
     *
     * This will destroy the items in the current vector, then copy
     * the items from @a other, potentially reusing the current allocation.
     *
     * @param other The vector to copy.
     */
    lean_vector& operator=(lean_vector const& other) noexcept
    {
        hi_return_on_self_assignment(other);

        hilet other_size = other.size();

        clear();
        hilet is_short = _reserve<false>(other_size);
        std::uninitialized_copy_n(other.begin(), other_size, begin());
        _set_size(other_size, is_short);

        return *this;
    }

    /** Move-assign a vector.
     *
     * This will swap the allocations between the current and @a other vector.
     * Or this will destroy the items in the current vector, then copy
     * the items from @a other, potentially reusing the current allocation.
     *
     * @param other The vector to move.
     */
    lean_vector& operator=(lean_vector&& other)
    {
        hi_return_on_self_assignment(other);

        if (not _is_short() and not other._is_short()) {
            std::swap(_ptr, other._ptr);
            std::swap(_end, other._end);
            std::swap(_cap, other._cap);

        } else {
            clear();
            shrink_to_fit();

            if (other._is_short()) {
                hilet other_size = other._short_size();
                std::uninitialized_move_n(other.begin(), other_size, _short_data());
                _set_short_size(other_size);

            } else {
                _ptr = std::exchange(other._ptr, nullptr);
                _end = std::exchange(other._end, nullptr);
                _cap = std::exchange(other._cap, nullptr);
            }
        }

        return *this;
    }

    void swap(lean_vector &other) noexcept
    {
        hilet this_is_short = this->_is_short();
        hilet other_is_short = other._is_short();
        if (this_is_short) {
            // By moving other to tmp, we make other short.
            lean_vector tmp = std::move(other);
            other = std::move(*this);
            *this = std::move(tmp);

        } else if (other_is_short) {
            // By moving this to tmp, we make this short.
            lean_vector tmp = std::move(*this);
            *this = std::move(other);
            other = std::move(tmp);

        } else {
            std::swap(this->_ptr, other._ptr);
            std::swap(this->_end, other._end);
            std::swap(this->_cap, other._cap);
        }
    }

    explicit lean_vector(size_type count)
    {
        if (count <= short_capacity) {
            std::uninitialized_value_construct_n(_short_data(), count);
            _set_short_size(count);

        } else {
            hilet is_short = _reserve<false>(count);
            hi_axiom(is_short == false);
            std::uninitialized_value_construct_n(_begin_data(is_short), count);
            _set_size(count, is_short);
        }
    }

    lean_vector(size_type count, value_type const& value)
    {
        if (count <= short_capacity) {
            std::uninitialized_fill_n(_short_data(), count, value);
            _set_short_size(count);

        } else {
            hilet is_short = _reserve<false>(count);
            hi_axiom(is_short == false);
            std::uninitialized_fill_n(_begin_data(is_short), count, value);
            _set_size(count, is_short);
        }
    }

    template<std::input_iterator It, std::input_iterator ItEnd>
    lean_vector(It first, ItEnd last)
    {
        insert(_short_data(), first, last);
    }

    /** Get a pointer to the first item.
     *
     * @return The pointer to the first item.
     */
    [[nodiscard]] pointer data() noexcept
    {
        return _begin_data(_is_short());
    }

    /** Get a const-pointer to the first item.
     *
     * @return The pointer to the first item.
     */
    [[nodiscard]] const_pointer data() const noexcept
    {
        return const_cast<lean_vector *>(this)->data();
    }

    /** Check if the vector is empty.
     *
     * @retval true The vector is empty.
     * @retval false The vector contains at least one item.
     */
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        if (_is_short()) {
            return _short_size() == 0;
        } else {
            return _ptr == _end;
        }
    }

    /** Get the number of items in the vector.
     *
     * @return The number of items in the vector.
     */
    [[nodiscard]] constexpr size_type size() const noexcept
    {
        if (_is_short()) {
            return _short_size();
        } else {
            hi_axiom(_ptr <= _end);
            return truncate<size_type>(_long_size());
        }
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept
    {
        auto a = get_allocator();
        return std::allocator_traits<allocator_type>::max_size(a);
    }

    /** Get the current capacity of the vector.
     *
     * @return The number of items that fit in the current allocation.
     */
    [[nodiscard]] size_t capacity() const noexcept
    {
        if (_is_short()) {
            return short_capacity;
        } else {
            return _long_capacity();
        }
    }

    /** Get a reference to an item in the vector.
     *
     * @param index The index to the item in the vector.
     * @return A reference to the item in the vector.
     * @throw std::out_of_range When index points beyond the size of the vector.
     */
    [[nodiscard]] reference at(size_type index)
    {
        hilet is_short = _is_short();
        hilet size_ = is_short ? _short_size() : _long_size();
        if (index < size_) {
            return _begin_data(is_short)[index];
        } else {
            throw std::out_of_range("lean_vector::at()");
        }
    }

    /** Get a const-reference to an item in the vector.
     *
     * @param index The index to the item in the vector.
     * @return A reference to the item in the vector.
     * @throw std::out_of_range When index points beyond the size of the vector.
     */
    [[nodiscard]] const_reference at(size_type index) const
    {
        return const_cast<lean_vector *>(this)->at(index);
    }

    /** Get a reference to an item in the vector.
     *
     * @note It is undefined-behavior if the index is beyond the size of the vector.
     * @param index The index to the item in the vector.
     * @return A reference to the item in the vector.
     */
    [[nodiscard]] reference operator[](size_type index) noexcept
    {
        hi_axiom(index < size());
        return _begin_data(_is_short())[index];
    }

    /** Get a const-reference to an item in the vector.
     *
     * @note It is undefined-behavior if the index is beyond the size of the vector.
     * @param index The index to the item in the vector.
     * @return A reference to the item in the vector.
     */
    [[nodiscard]] const_reference operator[](size_type index) const noexcept
    {
        return const_cast<lean_vector *>(this)->operator[](index);
    }

    /** Get a reference to the first item in the vector.
     *
     * @note It is undefined-behavior to call this function on an empty vector.
     * @return A reference to the first item in the vector.
     */
    [[nodiscard]] reference front() noexcept
    {
        hi_axiom(not empty());
        return *_begin_data(_is_short());
    }

    /** Get a const-reference to the first item in the vector.
     *
     * @note It is undefined-behavior to call this function on an empty vector.
     * @return A reference to the first item in the vector.
     */
    [[nodiscard]] const_reference front() const noexcept
    {
        return const_cast<lean_vector *>(this)->front();
    }

    /** Get a reference to the last item in the vector.
     *
     * @note It is undefined-behavior to call this function on an empty vector.
     * @return A reference to the last item in the vector.
     */
    [[nodiscard]] reference back() noexcept
    {
        hi_axiom(not empty());
        return *(_end_data(_is_short()) - 1);
    }

    /** Get a const-reference to the last item in the vector.
     *
     * @note It is undefined-behavior to call this function on an empty vector.
     * @return A reference to the last item in the vector.
     */
    [[nodiscard]] const_reference back() const noexcept
    {
        return const_cast<lean_vector *>(this)->back();
    }

    /** Get an iterator to the first item in the vector.
     *
     * @return A iterator to the first item in the vector.
     */
    [[nodiscard]] iterator begin() noexcept
    {
        return data();
    }

    /** Get an const-iterator to the first item in the vector.
     *
     * @return A const-iterator to the first item in the vector.
     */
    [[nodiscard]] const_iterator begin() const noexcept
    {
        return _begin_data(_is_short());
    }

    /** Get an const-iterator to the first item in the vector.
     *
     * @return A const-iterator to the first item in the vector.
     */
    [[nodiscard]] const_iterator cbegin() const noexcept
    {
        return begin();
    }

    /** Get an iterator beyond the last item in the vector.
     *
     * @return A iterator beyond the last item in the vector.
     */
    [[nodiscard]] iterator end() noexcept
    {
        return _end_data(_is_short());
    }

    /** Get an const-iterator beyond the last item in the vector.
     *
     * @return A const-iterator beyond the last item in the vector.
     */
    [[nodiscard]] const_iterator end() const noexcept
    {
        return const_cast<lean_vector *>(this)->end();
    }

    /** Get an const-iterator beyond the last item in the vector.
     *
     * @return A const-iterator beyond the last item in the vector.
     */
    [[nodiscard]] const_iterator cend() const noexcept
    {
        return end();
    }

    /** Remove all items from the vector.
     *
     * The allocation of the items remains.
     */
    void clear() noexcept
    {
        hilet is_short = _is_short();
        std::destroy(_begin_data(is_short), _end_data(is_short));
        _set_size(0, is_short);
    }

    /** Reserve capacity for items.
     *
     * This will create a new allocation when @a new_capacity is larger than the current capacity
     * and move the items from the previous allocation.
     *
     * @param new_capacity The new capacity that the vector should take.
     * @note This function is a no-op when the @a new_capacity is smaller than the current capacity.
     */
    void reserve(size_type new_capacity)
    {
        [[maybe_unused]] hilet is_short = _reserve<false>(new_capacity);
    }

    /** Shrink the allocation to fit the current number of items.
     *
     * If the current allocation is larger than the number of items in the vector,
     * then a new allocation is created to exactly fit the number of items and
     * the items are moved to the new allocation.
     *
     * @note This function is a no-op if the current allocation is the same as the number of items.
     */
    void shrink_to_fit()
    {
        if (_is_short()) {
            return;
        }

        hilet old_ptr = _ptr;
        hilet old_size = size();
        hilet old_capacity = capacity();

        if (old_size <= short_capacity) {
            _set_short_size(old_size);
        } else {
            auto a = get_allocator();
            _ptr = std::allocator_traits<allocator_type>::allocate(a, old_size);
            _end = _ptr + old_size;
            _cap = _ptr + old_size;
        }

        std::uninitialized_move_n(old_ptr, old_size, begin());
        std::destroy_n(old_ptr, old_size);
        auto a = get_allocator();
        std::allocator_traits<allocator_type>::deallocate(a, old_ptr, old_capacity);
    }

    /** Construct in-place a new item.
     *
     * If the item is not placed at the end, the item will be moved to the
     * correct position after it is created at the end.
     *
     * @param pos The position where the item will be created.
     * @param args The arguments to pass to the constructor of the new item.
     * @return An iterator pointing to the newly inserted item.
     */
    template<typename... Args>
    iterator emplace(const_iterator pos, Args&&...args)
    {
        hilet index = pos - begin();
        hilet new_size = size() + 1;
        hilet is_short = _reserve<true>(new_size);
        pos = begin() + index;

        std::construct_at(_end_data(is_short), std::forward<Args>(args)...);
        _set_size(new_size, is_short);

        std::rotate(pos, end() - 1, end());
        return pos;
    }

    /** Insert a new item.
     *
     * If the item is not placed at the end, the item will be moved to the
     * correct position after it is copied to the end.
     *
     * @param pos The position where the item will be inserted.
     * @param value The value to copy into the vector.
     * @return An iterator pointing to the newly inserted item.
     */
    iterator insert(const_iterator pos, value_type const& value)
    {
        hilet index = pos - begin();
        hilet new_size = size() + 1;
        hilet is_short = _reserve<true>(new_size);
        pos = begin() + index;

        std::uninitialized_copy_n(std::addressof(value), 1, _end_data(is_short));
        _set_size(new_size, is_short);

        std::rotate(pos, end() - 1, end());
        return pos;
    }

    /** Insert a new item.
     *
     * If the item is not placed at the end, the item will be moved to the
     * correct position after it is moved to the end.
     *
     * @param pos The position where the item will be inserted.
     * @param value The value to move into the vector.
     * @return An iterator pointing to the newly inserted item.
     */
    iterator insert(const_iterator pos, value_type&& value)
    {
        hilet index = pos - begin();
        hilet new_size = size() + 1;
        hilet is_short = _reserve<true>(new_size);
        pos = begin() + index;

        std::uninitialized_move_n(std::addressof(value), 1, _end_data(is_short));
        _set_size(new_size, is_short);

        std::rotate(pos, end() - 1, end());
        return pos;
    }

    /** Insert new items.
     *
     * If the items are not placed at the end, the items will be moved to the
     * correct position after they have been copied to the end.
     *
     * @param pos The position where the items will be inserted.
     * @param first An iterator to the first item to copy.
     * @param last An iterator to one beyond the last item to copy.
     * @return An iterator pointing to the newly inserted item.
     */
    template<std::input_iterator It, std::input_iterator ItEnd>
    iterator insert(const_iterator pos, It first, ItEnd last)
    {
        if constexpr (requires { std::distance(first, last); }) {
            auto n = std::distance(first, last);

            hilet index = pos - begin();
            hilet new_size = size() + n;
            hilet is_short = _reserve<true>(new_size);
            auto new_pos = begin() + index;

            std::uninitialized_move_n(first, n, _end_data(is_short));
            _set_size(new_size, is_short);

            std::rotate(new_pos, end() - n, end());
            return new_pos;

        } else {
            for (auto it = first; it != last; ++it) {
                pos = insert(pos, *it) + 1;
            }
        }
    }

    /** Erase an item at position.
     *
     * Erases an item at a position. Items beyond the position
     * will be moved.
     *
     * This function will not change the allocation and iterators up
     * to the erased item remain valid.
     *
     * @note It is undefined behavior to pass a iterator not belonging to this vector.
     * @param pos An iterator pointing to the item to be removed.
     * @return An iterator to the item after the removed item, or end().
     */
    iterator erase(const_iterator pos)
    {
        hi_axiom(pos >= begin() and pos <= end());
        auto new_pos = begin() + std::distance(cbegin(), pos);
        std::rotate(new_pos, new_pos + 1, end());

        hilet is_short = _is_short();
        _set_size(size() - 1, is_short);
        std::destroy_at(_end_data(is_short));
        return new_pos;
    }

    /** Erase an items.
     *
     * Erases an items between @a first and @a last. Items beyond the position
     * will be moved.
     *
     * This function will not change the allocation and iterators up
     * to the erased items remain valid.
     *
     * @param pos An iterator pointing to the item to be removed.
     * @return An iterator to the item after the removed items, or end().
     */
    iterator erase(const_iterator first, const_iterator last)
    {
        hilet n = std::distance(first, last);
        std::rotate(first, last, end());
        std::destroy(end() - n, end());
        _set_size(size() - n);
        return first;
    }

    /** In-place construct an item at the end of the vector.
     *
     * The item is directly constructed at the end of the vector.
     *
     * @param args The arguments passed to the constructor.
     * @return A reference to the newly constructed item in the vector.
     */
    template<typename... Args>
    reference emplace_back(Args&&...args)
    {
        hilet new_size = size() + 1;
        hilet is_short = _reserve<true>(new_size);
        auto obj = std::construct_at(_end_data(is_short), std::forward<Args>(args)...);
        _set_size(new_size, is_short);
        return *obj;
    }

    /** Copy an item to the end of the vector.
     *
     * @param value The value to copy to the vector.
     */
    void push_back(value_type const& value)
    {
        hilet new_size = size() + 1;
        hilet is_short = _reserve<true>(new_size);
        std::uninitialized_copy_n(std::addressof(value), 1, _end_data(is_short));
        _set_size(new_size, is_short);
    }

    /** Move an item to the end of the vector.
     *
     * @param value The value to move to the vector.
     */
    void push_back(value_type&& value)
    {
        hilet new_size = size() + 1;
        hilet is_short = _reserve<true>(new_size);
        std::uninitialized_move_n(std::addressof(value), 1, _end_data(is_short));
        _set_size(new_size, is_short);
    }

    /** Remove the last item from the vector.
     *
     * This function destroys the last item from the vector.
     *
     * @note It is undefined behavior to call this function on an empty vector.
     */
    void pop_back()
    {
        hi_axiom(not empty());

        hilet is_short = _is_short();
        _set_size(size() - 1, is_short);
        std::destroy_at(_end_data(is_short));
    }

    /** Resize a vector.
     *
     * When the @a new_size is larger than the current size a new allocation
     * may be created and the new-items beyond the old size are default constructed.
     *
     * When the @a new_size is smaller than the current size the allocation remains
     * and the old-items beyond @a new_size are destroyed.
     *
     * @param new_size The new size of the vector.
     */
    void resize(size_type new_size)
    {
        if (hilet old_size = size(); new_size > old_size) {
            hilet n = new_size - old_size;
            hilet is_short = _reserve<true>(new_size);

            std::uninitialized_value_construct_n(_end_data(is_short), n);
            _set_size(new_size, is_short);

        } else {
            hilet is_short = _is_short();
            hilet n = old_size - new_size;
            std::destroy(_end_data(is_short) - n, _end_data(is_short));
            _set_size(new_size, is_short);
        }
    }

    /** Resize a vector.
     *
     * When the @a new_size is larger than the current size a new allocation
     * may be created and the new-items beyond the old size are copied from @a value.
     *
     * When the @a new_size is smaller than the current size the allocation remains
     * and the old-items beyond @a new_size are destroyed.
     *
     * @param new_size The new size of the vector.
     * @param value The value of the items being optionally created.
     */
    void resize(size_type new_size, value_type const& value)
    {
        if (hilet old_size = size(); new_size > old_size) {
            hilet n = new_size - old_size;
            hilet is_short = _reserve<true>(new_size);

            std::uninitialized_fill_n(_end_data(is_short), n, value);
            _set_size(new_size, is_short);

        } else {
            hilet is_short = _is_short();
            hilet n = old_size - new_size;
            std::destroy(end() - n, end());
            _set_size(new_size, is_short);
        }
    }

    /** Compare two vectors.
     *
     * @param lhs The left-hand-side vector.
     * @param rhs The right-hand-side vector.
     * @retval true If both vectors contain the same items.
     */
    friend bool operator==(lean_vector const& lhs, lean_vector const& rhs) noexcept
    {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    /** Compare two vectors lexicographically.
     *
     * @param lhs The left-hand-side vector.
     * @param rhs The right-hand-side vector.
     * @return The three-way result of the lexicographical compare.
     */
    friend auto operator<=>(lean_vector const& lhs, lean_vector const& rhs) noexcept
    {
        return std::lexicographical_compare_three_way(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }

    /** Erase items of a value from a vector.
     *
     * @param c The vector with items
     * @param value The value to remove from the vector.
     * @return The number of items removed.
     */
    friend size_type erase(lean_vector& c, value_type const& value)
    {
        auto it = std::remove(c.begin(), c.end(), value);
        auto r = std::distance(it, c.end());
        c.erase(it, end());
        return r;
    }

    /** Erase items of a value from a vector.
     *
     * @param c The vector with items
     * @param pred A predicate function of the form `bool(value_type const &)`.
     * @return The number of items removed.
     */
    template<typename Pred>
    friend size_type erase(lean_vector& c, Pred pred)
    {
        auto it = std::remove(c.begin(), c.end(), pred);
        auto r = std::distance(it, c.end());
        c.erase(it, end());
        return r;
    }

private:
    pointer _ptr = nullptr;
    pointer _end = nullptr;
    pointer _cap = nullptr;

    [[nodiscard]] constexpr bool _is_short() const noexcept
    {
        if constexpr (std::endian::native == std::endian::little) {
            if (_ptr == nullptr) {
                return true;
            } else {
                return to_bool(to_int(_ptr) & 1);
            }
        } else {
            if (_cap == nullptr) {
                return true;
            } else {
                return to_bool(to_int(_cap) & 1);
            }
        }
    }

    [[nodiscard]] pointer _short_data() const noexcept
    {
        void *this_ = const_cast<lean_vector *>(this);
        if constexpr (std::endian::native == std::endian::little) {
            return std::launder(reinterpret_cast<pointer>(this_) + 1);
        } else {
            return std::launder(reinterpret_cast<pointer>(this_));
        }
    }

    [[nodiscard]] pointer _long_data() const noexcept
    {
        return _ptr;
    }

    [[nodiscard]] constexpr size_type _short_size() const noexcept
    {
        if constexpr (std::endian::native == std::endian::little) {
            if (_ptr == nullptr) {
                return 0;
            } else {
                return (to_int(_ptr) >> 1) & 0x1f;
            }
        } else {
            if (_cap == nullptr) {
                return 0;
            } else {
                return (to_int(_cap) >> 1) & 0x1f;
            }
        }
    }

    [[nodiscard]] constexpr size_type _long_size() const noexcept
    {
        hi_axiom(_ptr <= _end);
        return _end - _ptr;
    }

    [[nodiscard]] constexpr size_type _long_capacity() const noexcept
    {
        hi_axiom(_ptr <= _cap);
        return _cap - _ptr;
    }

    void _set_short_size(size_t new_size) noexcept
    {
        constexpr auto mask = ~intptr_t{0xff};

        new_size <<= 1;
        new_size |= 1;
        if constexpr (std::endian::native == std::endian::little) {
            _ptr = to_ptr<pointer>((to_int(_ptr) & mask) | new_size);
        } else {
            _cap = to_ptr<pointer>((to_int(_cap) & mask) | new_size);
        }
    }

    void _set_size(size_t new_size, bool is_short) noexcept
    {
        if (is_short) {
            hi_axiom(new_size <= short_capacity);
            _set_short_size(new_size);
        } else {
            _end = _ptr + new_size;
        }
    }

    [[nodiscard]] pointer _begin_data(bool is_short) const noexcept
    {
        if (is_short) {
            return _short_data();
        } else {
            return _long_data();
        }
    }

    [[nodiscard]] pointer _end_data(bool is_short) const noexcept
    {
        if (is_short) {
            return _short_data() + _short_size();
        } else {
            return _end;
        }
    }

    /** Reserve.
     *
     * @return Current reservation is short.
     */
    template<bool ForInsert>
    [[nodiscard]] bool _reserve(size_type new_capacity)
    {
        hilet old_is_short = _is_short();
        hilet old_capacity = old_is_short ? short_capacity : _long_capacity();
        if (new_capacity <= old_capacity) {
            [[likely]] return old_is_short;
        }

        if constexpr (ForInsert) {
            auto next_capacity = old_capacity + old_capacity / 2;
            if (new_capacity > next_capacity) {
                next_capacity = new_capacity + new_capacity / 2;
            }
            new_capacity = next_capacity;
        }

        hilet old_size = old_is_short ? _short_size() : _long_size();
        hilet old_ptr = old_is_short ? _short_data() : _ptr;

        auto a = get_allocator();
        hilet new_ptr = std::allocator_traits<allocator_type>::allocate(a, new_capacity);

        std::uninitialized_move_n(old_ptr, old_size, new_ptr);
        std::destroy_n(old_ptr, old_size);

        _ptr = new_ptr;
        _end = new_ptr + old_size;
        _cap = new_ptr + new_capacity;

        if (not old_is_short) {
            std::allocator_traits<allocator_type>::deallocate(a, old_ptr, old_capacity);
        }
        return false;
    }
};

}} // namespace hi::v1