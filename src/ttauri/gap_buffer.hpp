
#include "required.hpp"
#include <memory>

namespace tt {

template<typename T>
class gab_buffer_iterator;

/** Gap Buffer
 * This container is simular to a std::vector, optimized
 * for repeated insertions and deletion at the same position.
 *
 * This container is especially useful for text editing where
 * inserts and deletes are hapening at a cursor.
 *
 * Like a std::vector a gap_buffer has extra capacity to do
 * insertion without needing to reallocate, however this capacity
 * can be located anywhere in the allocated memory in a single
 * continues region called the gap.
 *
 * When inserting/deleting data in the gab_buffer the gap will
 * move to this location.
 */
template<typename T, typename Allocator=std::allocator<T>>
class gab_buffer {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = typename std::vector<T>::size_type; 
    using difference_type = typename std::vector<T>::difference_type; 
    using reference = typename std::vector<T>::reference; 
    using const_reference = typename std::vector<T>::const_reference; 
    using pointer = typename std::vector<T>::pointer; 
    using const_pointer = typename std::vector<T>::const_pointer; 
    using iterator = gab_buffer_iterator<T>;
    using const_iterator = gab_buffer_iterator<T> const;

    // By how much the buffer should grow when size() == capacity().
    static constexpr ssize_t grow_size = 256;

    /** Construct an empty buffer.
     */
    gap_buffer(allocator_type const &allocator = allocator_type{}) noexcept :
        _allocator(allocator), _ptr(nullptr), _size(0), _gap_offset(0), _gap_size(0) { }

    /** Copy constructor.
     * Allocates memory and copies all items from other into this.
     */
    gap_buffer(gap_buffer const &other) noexcept :
        _allocator(other._allocator), _ptr(nullptr), _size(other._size), _gap_offset(other._gap_offset), _gap_size(other._gap_size)
    {
        tt_assume(&other != this);

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
    gap_buffer &operator=(gap_buffer const &other) noexcept :
    {
        tt_assume(&other != this);

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
                _size = other.size() + grow_size;
                _ptr = _allocator.allocate(_size);
                _gap_offset = other._gap_offset;
                _gap_size = other._gap_size + grow_size;

                placement_copy(other.left_begin(), other.left_end(), left_begin());        
                placement_copy(other.right_begin(), other.right_end(), right_begin());        
            }
        }
    }

    /** Move constructor.
     * This constructor will move the allocation of the other gap_buffer.
     */
    gap_buffer(gap_buffer &&other) noexcept :
        _allocator(other._allocator), _ptr(other._ptr), _size(other._size), _gap_offset(other._gap_offset), _gap_size(other._gap_size)
    {
        other._ptr = nullptr;
        other._size = 0;
        other._gap_offset = 0;
        other._gap_size = 0;
    }

    /** Move assignement operator.
     * This functional will allocate its own buffer and move the items from other.
     */
    gap_buffer &operator=(gap_buffer &&other) noexcept
    {
        tt_assume(&other != this);

        clear();
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
                _size = other.size() + grow_size;
                _ptr = _allocator.allocate(_size);
                _gap_offset = other._gap_offset;
                _gap_size = other._gap_size + grow_size;

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
    ~gab_buffer()
    {
        clear();
        if (_ptr) {
            _allocator.deallocate(_ptr, _size);
        }
    }

    /** Index operator.
     * Return a reference to the item at index.
     *
     * @param index The index in the buffer.
     * @return A reference to the item in the buffer.
     */
    reference operator[](size_t index) noexcept
    {
        return *get_pointer(index);
    }

    /** Index operator.
     * Return a reference to the item at index.
     *
     * @param index The index in the buffer.
     * @return A reference to the item in the buffer.
     */
    const_reference operator[](size_t index) const noexcept
    {
        return *get_pointer(index);
    }

    /** Get item to reference at.
     * 
     * @throw std::out_of_range Thrown when index is out of range.
     * @param index The index in the buffer.
     * @return A reference to the item in the buffer.
     */
    reference at(size_t index)
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
    const_reference at(size_t index) const
    {
        if (i < size()) {
            throw std::out_of_range("gap_buffer::at");
        }
        return (*this)[index];
    }

    reference front() noexcept
    {
        tt_assume(size() != 0);
        return *(this)[0];
    }

    const_reference front() const noexcept
    {
        tt_assume(size() != 0);
        return *(this)[0];
    }

    reference back() noexcept
    {
        tt_assume(size() != 0);
        return *(this)[size() - 1];
    }

    const_reference back() const noexcept
    {
        tt_assume(size() != 0);
        return *(this)[size() - 1];
    }

    void pop_back() noexcept
    {
        tt_assume(size() != 0);
        erase(end() - 1);
    }

    void pop_front() noexcept
    {
        tt_assume(size() != 0);
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

    [[nodiscard]] size_t capacity() const noexcept
    {
        return _size;
    }

    void reserve(size_t new_capacity) const noexcept
    {
        auto extra_capacity = static_cast<ssize_t>(new_capacity) - capacity();
        if (extra_capacity <= 0) {
            return;
        }

        // Add the extra_capacity to the end of the gap.
        // LLL...RRR
        // LLL....RRR
        auto new_ptr = _allocator.allocate(new_capacity);
        auto new_size = _size + extra_capacity;
        auto new_gap_offset = _gap_offset;
        auto new_gap_size = _gap_size + extra_capacity;

        if (_ptr != nullptr) {
            auto new_left_begin = new_ptr;
            auto new_right_begin = new_ptr + new_gap_offset + new_gap_size;
            placement_move(left_begin(), left_end(), new_left_begin);
            placement_move(right_begin(), right_end(), new_right_begin);
            _allocate.deallocate();
        }

        _ptr = new_ptr;
        _gap_offset = new_gap_offset;
        _gap_size = new_gap_size;
        _size = new_size;
    }

    size_t size() const noexcept
    {
        return left_size() + right_size();
    }

    iterator begin() noexcept;

    const_iterator begin() const noexcept;

    iterator cbegin() const noexcept
    {
        return begin();
    }

    iterator end() noexcept;
    
    const_iterator end() const noexcept;

    iterator cend() const noexcept
    {
        return end();
    }

    template<typename... Args>
    void emplace_back(Args &&... args) noexcept
    {
        set_gap_offset(_size);
        grow_to_insert(1);

        auto p = _ptr + _gap_offset;
        auto p_ = new (p) value_type(std::forward<Args>(args)...);
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
    void emplace_front(Args &&... args) noexcept
    {
        set_gap_offset(0);
        grow_to_insert(1);

        auto p = _ptr + _gap_offset + _gap_size - 1;
        auto p_ = new (p) value_type(std::forward<Args>(args)...);
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
    iterator emplace_before(iterator pos, Args &&... args) noexcept;

    iterator insert_before(iterator pos, value_type const &value) noexcept
    {
        emplace_before(value_type(value));
    }

    iterator insert_before(iterator pos, value_type &&value) noexcept
    {
        emplace_before(std::move(value));
    }

    /** Insert items
     * 
     * @param pos Location to insert before.
     * @param first The first item to insert.
     * @param last The one beyond last item to insert.
     * @return The iterator pointing to the first item inserted.
     */
    template<typename It>
    iterator insert_before(iterator pos, It first, It last) noexcept
    {
        auto it = last;
        while (it != first) {
            pos = insert_before(pos, *(--it));
        }
        return pos;
    }

    /** Place the gap after the position and emplace at the beginning of the gap.
     */
    template<typename... Args>
    iterator emplace_after(iterator pos, Args &&... args) noexcept;

    iterator insert_after(iterator pos, value_type const &value) noexcept
    {
        emplace_after(value_type(value));
    }

    iterator insert_after(iterator pos, value_type &&value) noexcept
    {
        emplace_after(std::move(value));
    }

    /** Insert items
     * 
     * @param pos Location to insert after.
     * @param first The first item to insert.
     * @param last The one beyond last item to insert.
     * @return The iterator pointing to the last item inserted.
     */
    template<typename It>
    iterator insert_after(iterator pos, It first, It last) noexcept
    {
        for (auto it = first; it != last; ++it) {
            pos = insert_after(pos, *it);
        }
        return pos;
    }

    /** Erase items
     * @param first Location of first item to remove.
     * @param last Location beyond last item to remove.
     * @return iterator pointing to the element past the removed item, or end().
     */
    iterator erase(iterator first, iterator last) noexcept;

    /** Erase item
     * @param pos Location of item to remove
     * @return iterator pointing to the element past the removed item, or end().
     */
    iterator erase(iterator pos) noexcept
    {
        return erase(pos, pos + 1);
    }


private:
    allocator_type _allocator;
    value_type *_ptr;
    ssize_t _size;
    ssize_t _gap_offset;
    ssize_t _gap_size;

    /** Grow the gap_buffer based on the size to be inserted.
     */
    void grow_to_insert(ssize_t n) const noexcept
    {
        if (capacity() - _size < n) {
            reserve(capacity() + n + grow_size);
        }
    }

    /** Get an offset in memory from the given item index.
     *
     * @param index The index of an item.
     * @return offset from _ptr to the item in memory.
     */
    [[nodiscard]] ssize_t get_offset(size_t index) const noexcept
    {
        tt_assume(index < size());
        if (index > _gap_offset) {
            index += _gap_size;
        }
        return index;
    }

    /** Get a pointer to the item.
     * 
     * @param index The index of an item.
     * @return Pointer to the item in memory.
     */
    pointer get_pointer(size_t index) noexcept
    {
        return std::launder(_ptr + get_offset(index));
    }

    /** Get a pointer to the item.
     * 
     * @param index The index of an item.
     * @return Pointer to the item in memory.
     */
    const_pointer get_pointer(size_t i) const noexcept
    {
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
        return _ptr + _gap_offset;
    }

    [[nodiscard]] value_type *left_end() noexcept
    {
        return _ptr + _gap_offset;
    }

    [[nodiscard]] value_type const *right_begin() const noexcept
    {
        return _pre + _gap_offset + _gap_size;
    }

    [[nodiscard]] value_type *right_begin() noexcept
    {
        return _pre + _gap_offset + _gap_size;
    }

    [[nodiscard]] value_type const *right_end() const noexcept
    {
        return _ptr + _size;
    }

    [[nodiscard]] value_type *right_end() noexcept
    {
        return _ptr + _size;
    }

    [[nodiscard]] value_type const *gap_begin() const noexcept
    {
        return _ptr + _gap_offset;
    }

    [[nodiscard]] value_type *gap_begin() noexcept
    {
        return _ptr + _gap_offset;
    }

    [[nodiscard]] value_type const *gap_end() const noexcept
    {
        return _ptr + _gap_offset + _gap_size;
    }

    [[nodiscard]] value_type *gap_end() noexcept
    {
        return _ptr + _gap_offset + _gap_size;
    }

    /** Move the start of the gap to a new location.
     */
    void set_gap_offset(ssize_t new_gap_offset) noexcept
    {
        if (new_gap_offset < _gap_offset) {
            // Move data left of the original gap to the end of the new gap.
            // LLL...RRR
            // LL...LRRR
            auto new_gap_begin = _ptr + new_gap_offset;
            placement_move(new_gap_begin, gap_begin(), new_gap_end());

        } else if (new_gap_offset > _gap_offset) {{
            // Move data right of the original gap to the begining of the new gap.
            // LLL...RRR
            // LLLR...RR
            auto new_gap_end = _ptr + new_gap_offset + _gap_size;
            placement_move(gap_end(), new_gap_end, gap_begin());
        }
        _gap_offset = new_gap_offset;
    }

    friend gab_buffer_iterator<T>;
};

/** A continues iterator over a gap_buffer.
 */
template<typename T>
class gab_buffer_iterator {
public:
    using difference_type typename gab_buffer<T>::difference_type;
    using value_type = typename gab_buffer<T>::value_type;
    using pointer = typename gab_buffer<T>::pointer;
    using reference = typename gab_buffer<T>::reference;
    using const_reference = typename gab_buffer<T>::reference const;
    using iterator_category = std::random_access_iterator_tag;

    ~iterator() noexcept = default;
    iterator(iterator const &) noexcept = default;
    iterator(iterator &&) noexcept = default;
    iterator &operator=(iterator const &) noexcept = default;
    iterator &operator=(iterator &&) noexcept = default;

    iterator(gab_buffer<value_type> *buffer, value_type *it) noexcept :
        _buffer(buffer), _ptr(it)
    {
    }

    reference operator*() noexcept
    {
        return _buffer[_index];
    };

    const_reference operator*() const noexcept
    { 
        return _buffer[_index];
    };

    reference operator[](size_t i) noexcept
    {
        return *(_ptr + i)
    }

    const_reference operator[](size_t i) const noexcept
    {
        return *(_ptr + i)
    }

    iterator &operator++() noexcept
    {
        ++_index;
        tt_assume(valid());
        return *this; 
    }

    iterator operator++(int) noexcept
    {
        auto tmp = *this;
        ++_index;
        tt_assume(valid());
        return tmp; 
    }

    iterator &operator--() noexcept
    {
        --_index;
        tt_assume(valid());
        return *this; 
    }

    iterator &operator--(int) noexcept
    {
        auto tmp = *this;
        --_index;
        tt_assume(valid());
        return tmp; 
    }

    iterator &operator+=(ssize_t n) noexcept
    {
        _index += n;
        tt_assume(valid());
        return *this; 
    }

    iterator &operator-=(ssize_t n) noexcept
    {
        _index -= n;
        tt_assume(valid());
        return *this; 
    }

    iterator operator-(ssize_t n) const noexcept
    {
        auto tmp = *this;
        return tmp -= n;
    }

    friend difference_type operator-(iterator const &lhs, iterator const &rhs) noexcept
    {
        tt_assume(lhs.valid(rhs));
        return lhs._index - rhs._index;
    }

    friend iterator operator+(iterator lhs, ssize_t rhs) noexcept
    {
        return lhs += rhs;
    }

    friend iterator operator+(size_t lhs, iterator rhs) noexcept
    {
        return rhs += lhs;
    }

    friend bool operator==(iterator const &lhs, iterator const &rhs) noexcept
    {
        tt_assume(lhs.valid(rhs));
        return lhs._index == rhs._index;
    }

    friend bool operator!=(iterator const &lhs, iterator const &rhs) noexcept
    {
        tt_assume(lhs.valid(rhs));
        return lhs._index != rhs._index;
    }

    friend bool operator<(iterator const &lhs, iterator const &rhs) noexcept
    {
        tt_assume(lhs.valid(rhs));
        return lhs._index < rhs._index;
    }

    friend bool operator>(iterator const &lhs, iterator const &rhs) noexcept
    {
        tt_assume(lhs.valid(rhs));
        return lhs._index < rhs._index;
    }

    friend bool operator<=(iterator const &lhs, iterator const &rhs) noexcept
    {
        tt_assume(lhs.valid(rhs));
        return lhs._index <= rhs._index;
    }

    friend bool operator>=(iterator const &lhs, iterator const &rhs) noexcept
    {
        tt_assume(lhs.valid(rhs));
        return lhs._index >= rhs._index;
    }

private:
    gap_buffer<value_type> *_buffer;
    ssize_t _index;

    [[nodiscard]] bool valid() const noexcept
    {
        return _buffer != nullptr && _index >= 0 && _index < std::ssize(_buffer);
    }

    [[nodiscard]] bool valid(iterator const &other) const noexcept
    {
        return valid() && other.valid() && _buffer == other._buffer;
    }
};

template<typename T>
[[nodiscard]] inline gab_buffer_iterator<T> gab_buffer<T>::begin() noexcept 
{
    return gab_buffer_iterator<T>(this, 0);
}

template<typename T>
[[nodiscard]] inline gab_buffer_iterator<T> const gab_buffer<T>::begin() const noexcept 
{
    return gab_buffer_iterator<T>(this, 0);
}

template<typename T>
[[nodiscard]] gab_buffer_iterator<T> gab_buffer<T>::end() noexcept 
{
    return gab_buffer_iterator<T>(this, _size);
}

template<typename T>
[[nodiscard]] gab_buffer_iterator<T> const gab_buffer<T>::end() const noexcept 
{
    return gab_buffer_iterator<T>(this, _size);
}

template<typename T, typename... Args>
gab_buffer_iterator<T> gab_buffer<T>::emplace_before(iterator pos, Args &&... args) noexcept;
{
    tt_assume(pos._buffer == this);
    set_gap_offset(pos._index);
    grow_to_insert(1);

    auto p = _ptr + _gap_offset + _gap_size - 1;
    auto p_ = new (p) value_type(std::forward<Args>(args)...);
    --_gap_size;
    return gap_buffer_iterator<T>(this, _gap_offset + _gap_size);
}

template<typename T, typename... Args>
gab_buffer_iterator<T> gab_buffer<T>::emplace_after(iterator pos, Args &&... args) noexcept;
{
    tt_assume(pos._buffer == this);
    set_gap_offset(pos._index + 1);
    grow_to_insert(1);

    auto p = _ptr + _gap_offset;
    auto p_ = new (p) value_type(std::forward<Args>(args)...);
    ++_gap_offset;
    --_gap_size;
    return gap_buffer_iterator<T>(this, _gap_offset);
}

template<typename T>
gab_buffer_iterator<T> gab_buffer<T>::erase(iterator first, iterator last) noexcept
{
    // place the gap before the first iterator, so that we can extend it.
    tt_assume(first._buffer == this);
    tt_assume(last._buffer == this);

    set_gap_offset(first._index)
    auto first_p = get_pointer(first._index);
    auto last_p = get_pointer(last._index);
    std::destroy(first_p, last_p);
    _gap_size += last_p - first_p;
}

}

