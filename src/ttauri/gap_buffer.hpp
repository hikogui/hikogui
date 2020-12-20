// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "assert.hpp"
#include "memory.hpp"
#include "cast.hpp"
#include <memory>
#include <algorithm>
#include <string>
#include <type_traits>

namespace tt {

template<typename T, typename Allocator>
class gap_buffer;

template<typename T>
class gap_buffer_iterator;

template<typename T, typename Allocator>
gap_buffer_iterator<T> make_gap_buffer_iterator(gap_buffer<T, Allocator> *buffer, ptrdiff_t index);

template<typename T, typename Allocator>
gap_buffer_iterator<T const> make_gap_buffer_iterator(gap_buffer<T, Allocator> const *buffer, ptrdiff_t index);

/** Gap Buffer
 * This container is similar to a std::vector, optimized
 * for repeated insertions and deletion at the same position.
 *
 * This container is especially useful for text editing where
 * inserts and deletes are happening at a cursor.
 *
 * Like a std::vector a gap_buffer has extra capacity to do
 * insertion without needing to reallocate, however this capacity
 * can be located anywhere in the allocated memory in a single
 * continues region called the gap.
 *
 * When inserting/deleting data in the gap_buffer the gap will
 * move to this location.
 */
template<typename T, typename Allocator = std::allocator<T>>
class gap_buffer {
public:
    static_assert(
        !std::is_const_v<T> && !std::is_volatile_v<T> && !std::is_reference_v<T>,
        "Type of a managing container can not be const, volatile nor a reference");
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = T &;
    using const_reference = T const &;
    using pointer = T *;
    using const_pointer = T const *;
    using iterator = gap_buffer_iterator<T>;
    using const_iterator = gap_buffer_iterator<T const>;

    /** Construct an empty buffer.
     */
    gap_buffer(allocator_type const &allocator = allocator_type{}) noexcept :
        _ptr(nullptr), _size(0), _gap_offset(0), _gap_size(0), _allocator(allocator)
    {
    }

    /** Construct a buffer with the given initializer list.
     */
    gap_buffer(std::initializer_list<T> init, allocator_type const &allocator = allocator_type{}) :
        _ptr(_allocator.allocate(init.size() + _grow_size)),
        _size(init.size() + _grow_size),
        _gap_offset(init.size()),
        _gap_size(_grow_size),
        _allocator(allocator)
    {
        placement_copy(std::begin(init), std::end(init), left_begin());
    }

    /** Copy constructor.
     * Allocates memory and copies all items from other into this.
     */
    gap_buffer(gap_buffer const &other) noexcept :
        _ptr(nullptr),
        _size(other._size),
        _gap_offset(other._gap_offset),
        _gap_size(other._gap_size),
        _allocator(other._allocator)
    {
        tt_axiom(&other != this);

        if (other._ptr != nullptr) {
            _ptr = _allocator.allocate(_size);
            placement_copy(other.left_begin(), other.left_end(), left_begin());
            placement_copy(other.right_begin(), other.right_end(), right_begin());
        }
    }

    /** Copy assignment.
     * This copies the items from the other buffer to this buffer.
     * This may reuse allocation of this buffer if it was allocated before.
     *
     * Complexity is linear with the number of items in other.
     */
    gap_buffer &operator=(gap_buffer const &other) noexcept
    {
        tt_axiom(&other != this);

        clear();
        if (_size >= other.size()) {
            // Reuse memory.
            _gap_offset = other._gap_offset;
            _gap_size = other._size - other.right_size();

            placement_copy(other.left_begin(), other.left_end(), left_begin());
            placement_copy(other.right_begin(), other.right_end(), right_begin());

        } else {
            // Deallocate previous memory.
            if (_ptr != nullptr) {
                _allocator.deallocate(_ptr, _size);
                _ptr = nullptr;
                _size = 0;
                _gap_offset = 0;
                _gap_size = 0;
            }

            // Allocate and copy date.
            if (other._ptr != nullptr) {
                _size = other.size() + _grow_size;
                _ptr = _allocator.allocate(_size);
                _gap_offset = other._gap_offset;
                _gap_size = other._gap_size + _grow_size;

                placement_copy(other.left_begin(), other.left_end(), left_begin());
                placement_copy(other.right_begin(), other.right_end(), right_begin());
            }
        }
    }

    /** Move constructor.
     * This constructor will move the allocation of the other gap_buffer.
     */
    gap_buffer(gap_buffer &&other) noexcept :
        _ptr(other._ptr),
        _size(other._size),
        _gap_offset(other._gap_offset),
        _gap_size(other._gap_size),
        _allocator(other._allocator)
    {
        other._ptr = nullptr;
        other._size = 0;
        other._gap_offset = 0;
        other._gap_size = 0;
    }

    /** Move assignment operator.
     * This functional will allocate its own buffer and move the items from other.
     */
    gap_buffer &operator=(gap_buffer &&other) noexcept
    {
        tt_axiom(&other != this);

        // Clear the data inside this.
        clear();

        if (_allocator == other._allocator) {
            // When allocators are the same we can simply swap.
            std::swap(_allocator, other._allocator);
            std::swap(_size, other._size);
            std::swap(_gap_offset, other._gap_pffset);
            std::swap(_gap_size, other._size);
            return *this;
        }

        if (_size >= other.size()) {
            // Reuse memory.
            _gap_offset = other._gap_offset;
            _gap_size = other._size - other.right_size();

            placement_move(other.left_begin(), other.left_end(), left_begin());
            placement_move(other.right_begin(), other.right_end(), right_begin());

        } else {
            // Deallocate previous memory.
            if (_ptr != nullptr) {
                _allocator.deallocate(_ptr, _size);
                _ptr = nullptr;
                _size = 0;
                _gap_offset = 0;
                _gap_size = 0;
            }

            // Allocate and copy date.
            if (other._ptr != nullptr) {
                _size = other.size() + _grow_size;
                _ptr = _allocator.allocate(_size);
                _gap_offset = other._gap_offset;
                _gap_size = other._gap_size + _grow_size;

                placement_move(other.left_begin(), other.left_end(), left_begin());
                placement_move(other.right_begin(), other.right_end(), right_begin());
            }
        }

        // All items where moved, but keep the memory allocated in other, for potential reuse.
        other._gap_offset = 0;
        other._gap_size = other._size;
    }

    /** Destructor.
     * Destroys all items and deallocates the buffer.
     */
    ~gap_buffer()
    {
        clear();
        if (_ptr != nullptr) {
            _allocator.deallocate(_ptr, _size);
        }
    }

    /** Index operator.
     * Return a reference to the item at index.
     *
     * @param index The index in the buffer.
     * @return A reference to the item in the buffer.
     */
    [[nodiscard]] reference operator[](size_type index) noexcept
    {
        tt_axiom(index < size());
        return *get_pointer(index);
    }

    /** Index operator.
     * Return a reference to the item at index.
     *
     * @param index The index in the buffer.
     * @return A reference to the item in the buffer.
     */
    [[nodiscard]] const_reference operator[](size_type index) const noexcept
    {
        tt_axiom(index < size());
        return *get_pointer(index);
    }

    /** Get item to reference at.
     *
     * @throw std::out_of_range Thrown when index is out of range.
     * @param index The index in the buffer.
     * @return A reference to the item in the buffer.
     */
    [[nodiscard]] reference at(size_type index)
    {
        if (index < size()) {
            throw std::out_of_range("gap_buffer::at");
        }
        return (*this)[index];
    }

    /** Get item to reference at.
     *
     * @throw std::out_of_range Thrown when index is out of range.
     * @param index The index in the buffer.
     * @return A reference to the item in the buffer.
     */
    [[nodiscard]] const_reference at(size_type index) const
    {
        if (index < size()) {
            throw std::out_of_range("gap_buffer::at");
        }
        return (*this)[index];
    }

    [[nodiscard]] reference front() noexcept
    {
        tt_axiom(size() != 0);
        return *(this)[0];
    }

    [[nodiscard]] const_reference front() const noexcept
    {
        tt_axiom(size() != 0);
        return *(this)[0];
    }

    [[nodiscard]] reference back() noexcept
    {
        tt_axiom(size() != 0);
        return *(this)[size() - 1];
    }

    [[nodiscard]] const_reference back() const noexcept
    {
        tt_axiom(size() != 0);
        return *(this)[size() - 1];
    }

    void pop_back() noexcept
    {
        tt_axiom(size() != 0);
        erase(end() - 1);
    }

    void pop_front() noexcept
    {
        tt_axiom(size() != 0);
        erase(begin());
    }

    /** Clears the buffer.
     * Destroys all items in the buffer.
     * This function will keep the memory allocated.
     *
     * After this object was move() this function will allow the
     * object to be reused.
     */
    void clear() noexcept
    {
        if (_ptr) {
            std::destroy(left_begin(), left_end());
            std::destroy(right_begin(), right_end());
            _gap_offset = 0;
            _gap_size = _size;
        }
    }

    [[nodiscard]] size_t size() const noexcept
    {
        return _size - _gap_size;
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return size() == 0;
    }

    [[nodiscard]] size_t capacity() const noexcept
    {
        return _size;
    }

    void reserve(size_t new_capacity) noexcept
    {
        ttlet extra_capacity = static_cast<ssize_t>(new_capacity) - capacity();
        if (extra_capacity <= 0) {
            return;
        }

        // Add the extra_capacity to the end of the gap.
        // LLL...RRR
        // LLL....RRR
        ttlet new_ptr = _allocator.allocate(new_capacity);
        ttlet new_size = _size + extra_capacity;
        ttlet new_gap_offset = _gap_offset;
        ttlet new_gap_size = _gap_size + extra_capacity;

        if (_ptr != nullptr) {
            ttlet new_left_begin = new_ptr;
            ttlet new_right_begin = new_ptr + new_gap_offset + new_gap_size;
            placement_move(left_begin(), left_end(), new_left_begin);
            placement_move(right_begin(), right_end(), new_right_begin);
            _allocator.deallocate(_ptr, _size);
        }

        _ptr = new_ptr;
        _gap_offset = new_gap_offset;
        _gap_size = new_gap_size;
        _size = new_size;
    }

    [[nodiscard]] iterator begin() noexcept
    {
        return make_gap_buffer_iterator(this, 0);
    }

    [[nodiscard]] const_iterator begin() const noexcept
    {
        return make_gap_buffer_iterator(this, 0);
    }

    [[nodiscard]] iterator cbegin() const noexcept
    {
        return make_gap_buffer_iterator(this, 0);
    }

    [[nodiscard]] iterator end() noexcept
    {
        return make_gap_buffer_iterator(this, narrow_cast<difference_type>(size()));
    }

    [[nodiscard]] const_iterator end() const noexcept
    {
        return make_gap_buffer_iterator(this, narrow_cast<difference_type>(size()));
    }

    [[nodiscard]] iterator cend() const noexcept
    {
        return make_gap_buffer_iterator(this, narrow_cast<difference_type>(size()));
    }

    template<typename... Args>
    void emplace_back(Args &&...args) noexcept
    {
        set_gap_offset(size());
        grow_to_insert(1);

        auto p = _ptr + _gap_offset;
        new (p) value_type(std::forward<Args>(args)...);
        ++_gap_offset;
        --_gap_size;
    }

    void push_back(value_type const &value) noexcept
    {
        return emplace_back(value);
    }

    void push_back(value_type &&value) noexcept
    {
        return emplace_back(std::move(value));
    }

    template<typename... Args>
    void emplace_front(Args &&...args) noexcept
    {
        set_gap_offset(0);
        grow_to_insert(1);

        auto p = _ptr + _gap_offset + _gap_size - 1;
        new (p) value_type(std::forward<Args>(args)...);
        --_gap_size;
    }

    void push_front(value_type const &value) noexcept
    {
        return emplace_front(value);
    }

    void push_front(value_type &&value) noexcept
    {
        return emplace_front(std::move(value));
    }

    /** Place the gap before the position and emplace at the end of the gap.
     */
    template<typename... Args>
    iterator emplace_before(iterator position, Args &&...args) noexcept
    {
        tt_axiom(position.buffer() == this);
        set_gap_offset(position.index());
        grow_to_insert(1);

        auto p = _ptr + _gap_offset + _gap_size - 1;
        new (p) value_type(std::forward<Args>(args)...);
        --_gap_size;
        return gap_buffer_iterator<T>(this, _gap_offset + _gap_size);
    }

    iterator insert_before(iterator position, value_type const &value) noexcept
    {
        return emplace_before(position, value_type(value));
    }

    iterator insert_before(iterator position, value_type &&value) noexcept
    {
        return emplace_before(position, std::move(value));
    }

    /** Insert items
     *
     * @param position Location to insert before.
     * @param first The first item to insert.
     * @param last The one beyond last item to insert.
     * @return The iterator pointing to the first item inserted.
     */
    template<typename It>
    iterator insert_before(iterator position, It first, It last) noexcept
    {
        auto it = last;
        while (it != first) {
            position = insert_before(position, *(--it));
        }
        return position;
    }

    /** Place the gap after the position and emplace at the beginning of the gap.
     */
    template<typename... Args>
    iterator emplace_after(iterator position, Args &&...args) noexcept
    {
        tt_axiom(position.buffer() == this);
        set_gap_offset(position.index() + 1);
        grow_to_insert(1);

        auto p = _ptr + _gap_offset;
        new (p) value_type(std::forward<Args>(args)...);
        ++_gap_offset;
        --_gap_size;
        return gap_buffer_iterator<T>(this, _gap_offset);
    }

    iterator insert_after(iterator position, value_type const &value) noexcept
    {
        return emplace_after(position, value_type(value));
    }

    iterator insert_after(iterator position, value_type &&value) noexcept
    {
        return emplace_after(position, std::move(value));
    }

    /** Insert items
     *
     * @param position Location to insert after.
     * @param first The first item to insert.
     * @param last The one beyond last item to insert.
     * @return The iterator pointing to the last item inserted.
     */
    template<typename It>
    iterator insert_after(iterator position, It first, It last) noexcept
    {
        for (auto it = first; it != last; ++it) {
            position = insert_after(position, *it);
        }
        return position;
    }

    /** Erase items
     * @param first Location of first item to remove.
     * @param last Location beyond last item to remove.
     * @return iterator pointing to the element past the removed item, or end().
     */
    iterator erase(iterator first, iterator last) noexcept
    {
        // place the gap before the first iterator, so that we can extend it.
        tt_axiom(first.buffer() == this);
        tt_axiom(last.buffer() == this);

        set_gap_offset(first.index());
        ttlet first_p = get_pointer(first.index());
        ttlet last_p = get_pointer(last.index());
        std::destroy(first_p, last_p);
        _gap_size += last_p - first_p;
        return make_gap_buffer_iterator(this, _gap_offset);
    }

    /** Erase item
     * @param position Location of item to remove
     * @return iterator pointing to the element past the removed item, or end().
     */
    iterator erase(iterator position) noexcept
    {
        return erase(position, position + 1);
    }

    [[nodiscard]] friend bool operator==(gap_buffer const &lhs, gap_buffer const &rhs) noexcept
    {
        if (lhs.size() != rhs.size()) {
            return false;
        } else {
            return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs));
        }
    }

    template<typename Container>
    [[nodiscard]] friend bool operator==(gap_buffer const &lhs, Container const &rhs) noexcept
    {
        if (lhs.size() != rhs.size()) {
            return false;
        } else {
            return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs));
        }
    }

    template<typename Container>
    [[nodiscard]] friend bool operator==(Container const &lhs, gap_buffer const &rhs) noexcept
    {
        if (lhs.size() != rhs.size()) {
            return false;
        } else {
            return std::equal(std::begin(lhs), std::end(lhs), std::begin(rhs));
        }
    }

private:
    // By how much the buffer should grow when size() == capacity().
    static constexpr difference_type _grow_size = 256;

    value_type *_ptr;
    difference_type _size;
    difference_type _gap_offset;
    difference_type _gap_size;
    [[no_unique_address]] allocator_type _allocator;

    [[nodiscard]] bool is_valid() const noexcept
    {
        return _size >= 0 && _gap_offset >= 0 && _gap_size >= 0 && _gap_offset + _gap_size <= _size &&
            (_ptr != nullptr || (_size == 0 && _gap_offset == 0 && _gap_size == 0));
    }

    /** Grow the gap_buffer based on the size to be inserted.
     */
    void grow_to_insert(size_type n) noexcept
    {
        tt_axiom(is_valid());
        tt_axiom(n >= 0);
        if (n > narrow_cast<size_type>(_gap_size)) [[unlikely]] {
            auto new_capacity = size() + n + narrow_cast<size_type>(_grow_size);
            reserve(ceil(new_capacity, hardware_constructive_interference_size));
        }
    }

    /** Get an offset in memory from the given item index.
     *
     * @param index The index of an item.
     * @return offset from _ptr to the item in memory.
     */
    [[nodiscard]] difference_type get_offset(difference_type index) const noexcept
    {
        tt_axiom(is_valid());
        tt_axiom(index >= 0 && index <= std::ssize(*this));
        if (index >= _gap_offset) {
            index += _gap_size;
        }
        return index;
    }

    /** Get a pointer to the item.
     *
     * @param index The index of an item.
     * @return Pointer to the item in memory.
     */
    pointer get_pointer(difference_type index) noexcept
    {
        tt_axiom(is_valid());
        tt_axiom(index >= 0);
        return std::launder(_ptr + get_offset(index));
    }

    /** Get a pointer to the item.
     *
     * @param index The index of an item.
     * @return Pointer to the item in memory.
     */
    const_pointer get_pointer(difference_type index) const noexcept
    {
        tt_axiom(is_valid());
        tt_axiom(index >= 0);
        return std::launder(_ptr + get_offset(index));
    }

    [[nodiscard]] value_type const *left_begin() const noexcept
    {
        return _ptr;
    }

    [[nodiscard]] value_type *left_begin() noexcept
    {
        return _ptr;
    }

    [[nodiscard]] value_type const *left_end() const noexcept
    {
        tt_axiom(is_valid());
        return _ptr + _gap_offset;
    }

    [[nodiscard]] value_type *left_end() noexcept
    {
        tt_axiom(is_valid());
        return _ptr + _gap_offset;
    }

    [[nodiscard]] value_type const *right_begin() const noexcept
    {
        tt_axiom(is_valid());
        return _ptr + _gap_offset + _gap_size;
    }

    [[nodiscard]] value_type *right_begin() noexcept
    {
        tt_axiom(is_valid());
        return _ptr + _gap_offset + _gap_size;
    }

    [[nodiscard]] value_type const *right_end() const noexcept
    {
        tt_axiom(is_valid());
        return _ptr + _size;
    }

    [[nodiscard]] value_type *right_end() noexcept
    {
        tt_axiom(is_valid());
        return _ptr + _size;
    }

    [[nodiscard]] value_type const *gap_begin() const noexcept
    {
        tt_axiom(is_valid());
        return _ptr + _gap_offset;
    }

    [[nodiscard]] value_type *gap_begin() noexcept
    {
        tt_axiom(is_valid());
        return _ptr + _gap_offset;
    }

    [[nodiscard]] value_type const *gap_end() const noexcept
    {
        tt_axiom(is_valid());
        return _ptr + _gap_offset + _gap_size;
    }

    [[nodiscard]] value_type *gap_end() noexcept
    {
        tt_axiom(is_valid());
        return _ptr + _gap_offset + _gap_size;
    }

    /** Move the start of the gap to a new location.
     */
    void set_gap_offset(difference_type new_gap_offset) noexcept
    {
        ttlet new_gap_begin = _ptr + new_gap_offset;
        ttlet new_gap_end = new_gap_begin + _gap_size;

        if (new_gap_offset < _gap_offset) {
            // Move data left of the original gap to the end of the new gap.
            // LLL...RRR
            // LL...LRRR
            placement_move_within_array(new_gap_begin, gap_begin(), new_gap_end);

        } else if (new_gap_offset > _gap_offset) {
            // Move data right of the original gap to the beginning of the new gap.
            // LLL...RRR
            // LLLR...RR
            placement_move_within_array(gap_end(), new_gap_end, gap_begin());
        }

        _gap_offset = new_gap_offset;
    }

    friend gap_buffer_iterator<T>;
};

/** A continues iterator over a gap_buffer.
 */
template<typename T>
class gap_buffer_iterator {
public:
    static_assert(
        !std::is_volatile_v<T> && !std::is_reference_v<T>,
        "Type of a managing container iterator can not be volatile nor a reference");

    using value_type = std::remove_cv_t<T>;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using pointer = T *;
    using const_pointer = T const *;
    using reference = T &;
    using const_reference = T const &;
    using iterator_category = std::random_access_iterator_tag;

    using gap_buffer_type = std::conditional_t<std::is_const_v<T>, gap_buffer<value_type> const, gap_buffer<value_type>>;

    ~gap_buffer_iterator() noexcept = default;
    gap_buffer_iterator(gap_buffer_iterator const &) noexcept = default;
    gap_buffer_iterator(gap_buffer_iterator &&) noexcept = default;
    gap_buffer_iterator &operator=(gap_buffer_iterator const &) noexcept = default;
    gap_buffer_iterator &operator=(gap_buffer_iterator &&) noexcept = default;

    gap_buffer_iterator(gap_buffer_type *buffer, difference_type index) noexcept : _buffer(buffer), _index(index) {}

    gap_buffer_type *buffer() const noexcept
    {
        return _buffer;
    }

    difference_type index() const noexcept
    {
        return _index;
    }

    reference operator*() noexcept
    {
        return (*_buffer)[narrow_cast<size_type>(_index)];
    }

    const_reference operator*() const noexcept
    {
        tt_axiom(is_valid());
        return (*_buffer)[narrow_cast<size_type>(_index)];
    }

    reference operator[](std::integral auto index) noexcept
    {
        return (*_buffer)[narrow_cast<size_type>(_index + narrow_cast<difference_type>(index))];
    }

    const_reference operator[](std::integral auto index) const noexcept
    {
        return (*_buffer)[narrow_cast<size_type>(_index + narrow_cast<difference_type>(index))];
    }

    gap_buffer_iterator &operator++() noexcept
    {
        ++_index;
        tt_axiom(is_valid());
        return *this;
    }

    gap_buffer_iterator operator++(int) noexcept
    {
        auto tmp = *this;
        ++_index;
        tt_axiom(is_valid());
        return tmp;
    }

    gap_buffer_iterator &operator--() noexcept
    {
        --_index;
        tt_axiom(is_valid());
        return *this;
    }

    gap_buffer_iterator &operator--(int) noexcept
    {
        auto tmp = *this;
        --_index;
        tt_axiom(is_valid());
        return tmp;
    }

    gap_buffer_iterator &operator+=(std::integral auto n) noexcept
    {
        _index += narrow_cast<difference_type>(n);
        tt_axiom(is_valid());
        return *this;
    }

    gap_buffer_iterator &operator-=(std::integral auto n) noexcept
    {
        _index -= narrow_cast<difference_type>(n);
        tt_axiom(is_valid());
        return *this;
    }

    gap_buffer_iterator operator-(std::integral auto n) const noexcept
    {
        auto tmp = *this;
        return tmp -= n;
    }

    friend gap_buffer_iterator operator+(gap_buffer_iterator lhs, std::integral auto rhs) noexcept
    {
        return lhs += rhs;
    }

    friend gap_buffer_iterator operator+(std::integral auto lhs, gap_buffer_iterator rhs) noexcept
    {
        return rhs += lhs;
    }

    template<typename R>
    requires(std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<R>>) friend difference_type
    operator-(gap_buffer_iterator const &lhs, gap_buffer_iterator<R> const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid(rhs));
        return lhs._index - rhs._index;
    }

    template<typename R>
    requires(std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<R>>) friend bool
    operator==(gap_buffer_iterator const &lhs, gap_buffer_iterator<R> const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid(rhs));
        return lhs._index == rhs._index;
    }

    template<typename R>
    requires(std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<R>>) friend bool
    operator!=(gap_buffer_iterator const &lhs, gap_buffer_iterator<R> const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid(rhs));
        return lhs._index != rhs._index;
    }

    template<typename R>
    requires(std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<R>>) friend bool
    operator<(gap_buffer_iterator const &lhs, gap_buffer_iterator<R> const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid(rhs));
        return lhs._index < rhs._index;
    }

    template<typename R>
    requires(std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<R>>) friend bool
    operator>(gap_buffer_iterator const &lhs, gap_buffer_iterator<R> const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid(rhs));
        return lhs._index < rhs._index;
    }

    template<typename R>
    requires(std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<R>>) friend bool
    operator<=(gap_buffer_iterator const &lhs, gap_buffer_iterator<R> const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid(rhs));
        return lhs._index <= rhs._index;
    }

    template<typename R>
    requires(std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<R>>) friend bool
    operator>=(gap_buffer_iterator const &lhs, gap_buffer_iterator<R> const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid(rhs));
        return lhs._index >= rhs._index;
    }

private:
    gap_buffer_type *_buffer;
    difference_type _index;

    [[nodiscard]] bool is_valid() const noexcept
    {
        return _buffer != nullptr && _index >= 0 && _index <= std::ssize(*_buffer);
    }

    template<typename O>
    requires(std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<O>>)
        [[nodiscard]] bool is_valid(gap_buffer_iterator<O> const &other) const noexcept
    {
        return is_valid() && other.is_valid() && _buffer == other._buffer;
    }
};

template<typename T, typename Allocator>
gap_buffer_iterator<T> make_gap_buffer_iterator(gap_buffer<T, Allocator> *buffer, ptrdiff_t index)
{
    return {buffer, index};
}

template<typename T, typename Allocator>
gap_buffer_iterator<T const> make_gap_buffer_iterator(gap_buffer<T, Allocator> const *buffer, ptrdiff_t index)
{
    return {buffer, index};
}

} // namespace tt
