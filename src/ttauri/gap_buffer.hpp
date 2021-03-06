// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

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
        _begin(nullptr), _end(nullptr), _gap_begin(nullptr), _gap_size(0), _allocator(allocator)
    {
    }

    /** Construct a buffer with the given initializer list.
     */
    gap_buffer(std::initializer_list<T> init, allocator_type const &allocator = allocator_type{}) :
        _begin(_allocator.allocate(init.size() + _grow_size)),
        _end(_begin + init.size() + _grow_size),
        _gap_begin(_begin + init.size()),
        _gap_size(_grow_size),
        _allocator(allocator)
    {
        placement_copy(std::begin(init), std::end(init), _begin);
    }

    /** Copy constructor.
     * Allocates memory and copies all items from other into this.
     */
    gap_buffer(gap_buffer const &other) noexcept :
        _begin(nullptr),
        _end(nullptr),
        _gap_begin(nullptr),
        _gap_size(0),
        _allocator(other._allocator)
    {
        tt_axiom(&other != this);
        tt_axiom(other.is_valid());

        if (other._ptr != nullptr) {
            _begin = _allocator.allocate(other.capacity());
            _end = _begin + other.capacity();
            _gap_begin = _begin + other.left_size();
            _gap_size = other._gap_size;
            
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
        if (&other == this) {
            return *this;
        }

        clear();
        if (_gap_size >= other.size()) {
            // Reuse memory.
            _gap_begin = _begin + other.left_size();
            _gap_size = capacity() - other.size();

            placement_copy(other.left_begin(), other.left_end(), left_begin());
            placement_copy(other.right_begin(), other.right_end(), right_begin());

        } else {
            // Deallocate previous memory.
            if (_begin != nullptr) {
                _allocator.deallocate(_begin, capacity());
                _begin = nullptr;
                _end = nullptr;
                _gap_begin = nullptr;
                _gap_size = 0;
            }

            // Allocate and copy date.
            if (other._begin != nullptr) {
                ttlet new_capacity = other.size() + _grow_size;

                _begin = _allocator.allocate(new_capacity);
                _end = _begin + new_capacity;
                _gap_begin = _begin + other.left_begin();
                _gap_size = new_capacity - other.size();

                placement_copy(other.left_begin(), other.left_end(), left_begin());
                placement_copy(other.right_begin(), other.right_end(), right_begin());
            }
        }
    }

    /** Move constructor.
     * This constructor will move the allocation of the other gap_buffer.
     */
    gap_buffer(gap_buffer &&other) noexcept :
        _begin(other._begin),
        _end(other._end),
        _gap_begin(other._gap_begin),
        _gap_size(other._gap_size),
        _allocator(other._allocator)
    {
        tt_axiom(&other != this);

        other._begin = nullptr;
        other._end = nullptr;
        other._gap_begin = nullptr;
        other._gap_size = 0;
    }

    /** Move assignment operator.
     * This functional will allocate its own buffer and move the items from other.
     */
    gap_buffer &operator=(gap_buffer &&other) noexcept
    {
        if (&other == this) {
            return *this;
        }

        // Clear the data inside this.
        clear();

        if (_allocator == other._allocator) {
            // When allocators are the same we can simply swap.
            std::swap(_begin, other._begin);
            std::swap(_end, other._end);
            std::swap(_gap_begin, other._gap_begin);
            std::swap(_gap_size, other._size);
            return *this;
        }

        if (capacity() >= other.size()) {
            // Reuse memory.
            _gap_begin = _begin + other.left_size;
            _gap_size = capacity() - other.size();

            placement_move(other.left_begin(), other.left_end(), left_begin());
            placement_move(other.right_begin(), other.right_end(), right_begin());

        } else {
            // Deallocate previous memory.
            if (_ptr != nullptr) {
                _allocator.deallocate(_begin, capacity());
                _begin = nullptr;
                _end = nullptr;
                _gap_begin = nullptr;
                _gap_size = 0;
            }

            // Allocate and copy date.
            if (other._begin != nullptr) {
                ttlet new_capacity = other.size() + _grow_size;

                _begin = _allocator.allocate(new_capacity);
                _end = _begin + new_capacity;
                _gap_begin = _begin + other.left_size()
                _gap_size = new_capacity - other.size();

                placement_move(other.left_begin(), other.left_end(), left_begin());
                placement_move(other.right_begin(), other.right_end(), right_begin());
            }
        }

        // All items where moved, but keep the memory allocated in other, for potential reuse.
        other._gap_begin = other._begin;
        other._gap_size = other.capacity();
    }

    /** Destructor.
     * Destroys all items and deallocates the buffer.
     */
    ~gap_buffer()
    {
        clear();
        if (_begin != nullptr) {
            _allocator.deallocate(_begin, capacity());
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
        return *get_pointer_from_index(index);
    }

    /** Index operator.
     * Return a reference to the ittem at index.
     *
     * @param index The index in the buffer.
     * @return A reference to the item in the buffer.
     */
    [[nodiscard]] const_reference operator[](size_type index) const noexcept
    {
        tt_axiom(index < size());
        return *get_pointer_from_index(index);
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
        return *get_pointer_from_index(index);
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
        return *get_pointer_from_index(index);
    }

    [[nodiscard]] reference front() noexcept
    {
        tt_axiom(size() != 0);
        return *get_pointer_from_it_ptr(_begin);
    }

    [[nodiscard]] const_referencei front() const noexcept
    {
        tt_axiom(size() != 0);
        return *get_pointer_from_it_ptr(_begin);
    }

    [[nodiscard]] reference back() noexcept
    {
        tt_axiom(size() != 0);
        return *get_pointer_from_it_ptr(_end - 1);
    }

    [[nodiscard]] const_reference back() const noexcept
    {
        tt_axiom(size() != 0);
        return *get_pointer_from_it_ptr(_end - 1);
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
            _gap_begin = _begin;
            _gap_size = capacity();
            _end = _begin + capacity();
        }
    }

    [[nodiscard]] size_t size() const noexcept
    {
        return static_cast<size_t>(_it_end - _begin);
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return size() == 0;
    }

    [[nodiscard]] size_t capacity() const noexcept
    {
        return size() + _gap_size;
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
        ttlet new_begin = _allocator.allocate(new_capacity);
        ttlet new_it_end = new_begin + size();
        ttlet new_gap_begin = new_begin + left_size();
        ttlet new_gap_size = new_capacity - size();

        if (_begin != nullptr) {
            placement_move(left_begin(), left_end(), new_begin);
            placement_move(right_begin(), right_end(), new_gap_begin + new_gap_size);
            _allocator.deallocate(_ptr, _size);
        }

        _begin = new_begin;
        _it_end = new_it_end;
        _gap_begin = new_gap_begin;
        _gap_size = new_gap_size;
    }

    [[nodiscard]] iterator begin() noexcept
    {
        return make_gap_buffer_iterator(this, _begin);
    }

    [[nodiscard]] const_iterator begin() const noexcept
    {
        return make_gap_buffer_iterator(this, _begin);
    }

    [[nodiscard]] iterator cbegin() const noexcept
    {
        return make_gap_buffer_iterator(this, _begin);
    }

    [[nodiscard]] iterator end() noexcept
    {
        return make_gap_buffer_iterator(this, _it_end);
    }

    [[nodiscard]] const_iterator end() const noexcept
    {
        return make_gap_buffer_iterator(this, _it_end);
    }

    [[nodiscard]] iterator cend() const noexcept
    {
        return make_gap_buffer_iterator(this, _it_end);
    }

    template<typename... Args>
    void emplace_back(Args &&...args) noexcept
    {
        set_gap_offset(_begin + size());
        grow_to_insert(1);

        new (_gap_begin) value_type(std::forward<Args>(args)...);
        ++_it_end;
        ++_gap_begin;
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
        set_gap_offset(_begin);
        grow_to_insert(1);

        new (_gap_begin ) value_type(std::forward<Args>(args)...);
        ++_it_end;
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

    /** Place the gap at the position and emplace at the end of the gap.
     * If an insert requires a reallocation (size() == capacity()) then all current
     * iterators become invalid.
     */
    template<typename... Args>
    iterator emplace_before(iterator position, Args &&...args) noexcept
    {
        tt_axiom(position.buffer() == this);
        set_gap_offset(position.it_ptr());
        grow_to_insert(1);

        new (right_begin() - 1) value_type(std::forward<Args>(args)...);
        ++_it_end;
        --_gap_size;
        return gap_buffer_iterator<T>(this, _gap_begin);
    }

    /** Place the gap at the position and emplace at the end of the gap.
     * If an insert requires a reallocation (size() == capacity()) then all current
     * iterators become invalid.
     */
    iterator insert_before(iterator position, value_type const &value) noexcept
    {
        return emplace_before(position, value_type(value));
    }

    /** Place the gap at the position and emplace at the end of the gap.
     * If an insert requires a reallocation (size() == capacity()) then all current
     * iterators become invalid.
     */
    iterator insert_before(iterator position, value_type &&value) noexcept
    {
        return emplace_before(position, std::move(value));
    }

    /** Insert items
     * If an insert requires a reallocation then all current
     * iterators become invalid.
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
     * If an insert requires a reallocation (size() == capacity()) then all current
     * iterators become invalid.
     */
    template<typename... Args>
    iterator emplace_after(iterator position, Args &&...args) noexcept
    {
        tt_axiom(position.buffer() == this);
        set_gap_offset(position.it_ptr() + 1);
        grow_to_insert(1);

        new (_gap_begin) value_type(std::forward<Args>(args)...);
        ++_it_end;
        ++_gap_begin;
        --_gap_size;
        return gap_buffer_iterator<T>(this, _gap_begin - 1);
    }

    /** Place the gap after the position and emplace at the beginning of the gap.
     * If an insert requires a reallocation (size() == capacity()) then all current
     * iterators become invalid.
     */
    iterator insert_after(iterator position, value_type const &value) noexcept
    {
        return emplace_after(position, value_type(value));
    }

    /** Place the gap after the position and emplace at the beginning of the gap.
     * If an insert requires a reallocation (size() == capacity()) then all current
     * iterators become invalid.
     */
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
        // place the gap after the last iterator, this way we can use the
        // it_ptr directly because we don't need to skip the gap.
        tt_axiom(first.buffer() == this);
        tt_axiom(last.buffer() == this);

        set_gap_offset(last.it_ptr());
        ttlet first_p = first.it_ptr();
        ttlet last_p = last.it_ptr();
        ttlet erase_size = last_p - first_p;

        std::destroy(first_p, last_p);
        _gap_begin = first_p;
        _gap_size += erase_size;
        _it_end -= erase_size;
        return make_gap_buffer_iterator(this, _gap_begin);
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

    /** Start of the memory array.
     */
    value_type *_begin;

    /** The end iterator.
     * To get a real pointer use `pointer_from_it()`
     */
    value_type *_it_end;

    /** Location in memory where the gap stars.
     */
    value_type *_gap_begin;

    /** The size of the gap.
     */
    size_type _gap_size;

    [[no_unique_address]] allocator_type _allocator;

    [[nodiscard]] bool is_valid() const noexcept
    {
        return
            (_begin == nullptr && _it_end == nullptr && _gap_begin == nullptr && _gap_size == 0) ||
            (_begin <= _gap_begin && _gap_begin <= _end);
    }

    /** Grow the gap_buffer based on the size to be inserted.
     */
    void grow_to_insert(size_type n) noexcept
    {
        tt_axiom(is_valid());
        if (n > _gap_size) [[unlikely]] {
            auto new_capacity = size() + n + narrow_cast<size_type>(_grow_size);
            reserve(ceil(new_capacity, hardware_constructive_interference_size));
        }
    }

    /** Get a pointer to the item.
     *
     * @param it_ptr The pointer from a gab_buffer_iterator.
     * @return Pointer to the item in memory.
     */
    const_pointer get_const_pointer_from_it(const_pointer it_ptr) const noexcept
    {
        tt_axiom(is_valid());
        tt_axion(it_ptr >= _begin && it_ptr <= _end);

        if (it_ptr < _gap_begin) {
            return it_ptr;
        } else {
            return it_ptr + _gap_size;
        }
    }

    pointer get_pointer_from_it(pointer it_ptr) const noexcept
    {
        return const_cast<pointer *>(get_const_pointer_from_it(it_ptr));
    }

    [[nodiscard]] value_type const *left_begin() const noexcept
    {
        tt_axiom(is_valid());
        return _begin;
    }

    [[nodiscard]] value_type *left_begin() noexcept
    {
        tt_axiom(is_valid());
        return _begin;
    }

    [[nodiscard]] value_type const *left_end() const noexcept
    {
        tt_axiom(is_valid());
        return _gap_begin;
    }

    [[nodiscard]] value_type *left_end() noexcept
    {
        tt_axiom(is_valid());
        return _gap_begin;
    }

    [[nodiscard]] size_type left_size() const noexcept
    {
        tt_axiom(is_valid());
        return static_cast<size_type>(_gap_begin - _begin);
    }

    [[nodiscard]] value_type const *right_begin() const noexcept
    {
        tt_axiom(is_valid());
        return _gap_begin + _gap_size;
    }

    [[nodiscard]] value_type *right_begin() noexcept
    {
        tt_axiom(is_valid());
        return _gap_begin + _gap_size;
    }

    [[nodiscard]] value_type const *right_end() const noexcept
    {
        tt_axiom(is_valid());
        return _it_end + _gap_size;
    }

    [[nodiscard]] value_type *right_end() noexcept
    {
        tt_axiom(is_valid());
        return _it_end + _gap_size;
    }

    [[nodiscard]] size_type right_size() const noexcept
    {
        tt_axiom(is_valid());
        return _it_end - _gap_begin;
    }


    /** Move the start of the gap to a new location.
     */
    void set_gap_offset(value_type *new_gap_begin) noexcept
    {
        tt_axiom(is_valid());
        if (new_gap_begin < _gap_begin) {
            // Move data left of the original gap to the end of the new gap.
            // LLL...RRR
            // LL...LRRR
            placement_move_within_array(new_gap_begin, _gap_begin, new_gap_begin + _gap_size);

        } else if (new_gap_begin > _gap_begin) {
            // Move data right of the original gap to the beginning of the new gap.
            // LLL...RRR
            // LLLR...RR
            placement_move_within_array(_gap_begin + _gap_size, new_gap_begin + _gap_size, _gap_begin);
        }

        _gap_begin = new_gap_begin;
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

    pointer it_ptr() const noexcept
    {
        return _it_ptr;
    }

    reference operator*() noexcept
    {
        tt_axiom(is_valid());
        return *(_buffer->get_pointer(_it_ptr));
    }

    const_reference operator*() const noexcept
    {
        tt_axiom(is_valid());
        return *(_buffer->get_pointer(_it_ptr));
    }

    reference operator[](std::integral auto index) noexcept
    {
        tt_axiom(is_valid());
        return *(_buffer->get_pointer(_it_ptr + index));
    }

    const_reference operator[](std::integral auto index) const noexcept
    {
        tt_axiom(is_valid());
        return *(_buffer->get_pointer(_it_ptr + index));
    }

    gap_buffer_iterator &operator++() noexcept
    {
        ++_it_ptr;
        tt_axiom(is_valid());
        return *this;
    }

    gap_buffer_iterator operator++(int) noexcept
    {
        auto tmp = *this;
        ++_it_ptr;
        tt_axiom(is_valid());
        return tmp;
    }

    gap_buffer_iterator &operator--() noexcept
    {
        --_it_ptr;
        tt_axiom(is_valid());
        return *this;
    }

    gap_buffer_iterator &operator--(int) noexcept
    {
        auto tmp = *this;
        --_it_ptr;
        tt_axiom(is_valid());
        return tmp;
    }

    gap_buffer_iterator &operator+=(difference_type n) noexcept
    {
        _it_ptr += n;
        tt_axiom(is_valid());
        return *this;
    }

    gap_buffer_iterator &operator-=(difference_type n) noexcept
    {
        _it_ptr -= n;
        tt_axiom(is_valid());
        return *this;
    }

    gap_buffer_iterator operator-(difference_type n) const noexcept
    {
        auto tmp = *this;
        return tmp -= n;
    }

    friend gap_buffer_iterator operator+(gap_buffer_iterator lhs, difference_type rhs) noexcept
    {
        return lhs += rhs;
    }

    friend gap_buffer_iterator operator+(difference_type lhs, gap_buffer_iterator rhs) noexcept
    {
        return rhs += lhs;
    }

    template<typename R>
    requires(std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<R>>) friend difference_type
    operator-(gap_buffer_iterator const &lhs, gap_buffer_iterator<R> const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid(rhs));
        return lhs._it_ptr - rhs._it_ptr;
    }

    template<typename R>
    requires(std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<R>>) friend bool
    operator==(gap_buffer_iterator const &lhs, gap_buffer_iterator<R> const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid(rhs));
        return lhs._it_ptr == rhs.it_ptr();
    }

    template<typename R>
    requires(std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<R>>) friend auto
    operator<=>(gap_buffer_iterator const &lhs, gap_buffer_iterator<R> const &rhs) noexcept
    {
        tt_axiom(lhs.is_valid(rhs));
        return lhs._it_ptr <=> rhs.it_ptr();
    }

private:
    gap_buffer_type *_buffer;
    value_type *_it_ptr;

    [[nodiscard]] bool is_valid() const noexcept
    {
        return _buffer != nullptr && _it_ptr >= _buffer->_begin && _it_ptr <= _buffer->_end;
    }

    template<typename O>
    requires(std::is_same_v<std::remove_cv_t<T>, std::remove_cv_t<O>>)
    [[nodiscard]] bool is_valid(gap_buffer_iterator<O> const &other) const noexcept
    {
        return is_valid() && other.is_valid() && _buffer == other._buffer;
    }
};

template<typename T, typename Allocator>
gap_buffer_iterator<T> make_gap_buffer_iterator(gap_buffer<T, Allocator> *buffer, T *it_ptr)
{
    return {buffer, it_ptr};
}

template<typename T, typename Allocator>
gap_buffer_iterator<T const> make_gap_buffer_iterator(gap_buffer<T, Allocator> const *buffer, T *it_ptr)
{
    return {buffer, it_ptr};
}

} // namespace tt
