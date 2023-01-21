


namespace hi {
inline namespace v1 {

/** Lookahead iterator.
 *
 * This iterator adapter takes a forward input iterator and adapts it so that
 * you can look ahead beyond the current position of the iterator. This
 * is useful when writing a parser. 
 */
template<size_t LookaheadCount, typename Iterator>
class lookahead_iterator {
public:
    constexpr static size_t max_size = LookaheadCount + 1;

    using iterator = Iterator;

    using value_type = iterator_type::value_type;
    using reference = iterator_type::reference;
    using const_reference = iterator_type::const_reference;
    using pointer = iterator_type::pointer;
    using const_pointer = iterator_type::const_pointer;

    constexpr lookahead_iterator() noexcept = default;
    constexpr lookahead_iterator(lookahead_iterator const &) noexcept = delete;
    constexpr lookahead_iterator(lookahead_iterator &&) noexcept = default;
    constexpr lookahead_iterator&operator=(lookahead_iterator const &) noexcept = delete;
    constexpr lookahead_iterator&operator=(lookahead_iterator &&) noexcept = default;

    constexpr explicit lookahead_iterator(forward_of<Iterator> auto &&it) noexcept : _it(hi_forward(it)), _size(0)
    {
        for (; _it != std::default_sentinel and _size != max_size; ++_it, ++_size) {
            _cache[i] = std::move(*it);
        }
    }

    /** The number of entries can be looked ahead.
     *
     * @return Number of entries that can be looked ahead, including the current entry.
     */
    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return _size;
    }

    /** Check if the iterator is at end.
     *
     * @retval true Iterator at end.
     */
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _size == 0;
    }

    constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] constexpr bool operator==(std::default_sentinel_t const &) const noexcept
    {
        return _size == 0;
    }
  
    /** Get a reference to an item at or beyond the iterator.
     *
     * @param i Index to lookahead, 0 means the current iterator, larger than zero is lookahead.
     * @return A reference to the item.
     */
    [[nodiscard]] const_reference operator[](size_t i) const noexcept
    {
        hi_axiom(i < _size);
        return _cache[i];
    }

    /** Get a reference to an item at or beyond the iterator.
     *
     * @param i Index to lookahead, 0 means the current iterator, larger than zero is lookahead.
     * @throws std::out_of_range when index beyond the lookahead buffer.
     * @return A reference to the item.
     */
    [[nodiscard]] const_reference at(size_t i) const
    {
        if (i < _size) {
            return _cache[i];
        } else {
            throw std::out_of_range("lookahead_iterator::at()");
        }
    }

    /** Get a reference to an item at or beyond the iterator.
     *
     * @param i Index to lookahead, 0 means the current iterator, larger than zero is lookahead.
     * @retval std::nullopt When the index points beyond the lookahead buffer.
     * @return A reference to the item.
     */
    [[nodiscard]] std::optional<value_type> next(size_t i = 1) const noexcept
    {
        if (i < _size) {
            return _cache[i];
        } else {
            return std::nullopt;
        }
    }

    /** Get a reference to the value at the iterator.
     */
    const_reference operator*() const noexcept
    {
        hi_axiom(_size != 0);
        return _cache[i];
    }

    /** Get a pointer to the value at the iterator.
     */
    const_pointer operator->() const noexcept
    {
        hi_axiom(_size != 0);
        return _cache->data();
    }

    /** Increment the iterator.
     */
    lookahead_iterator &operator++() noexcept
    {
        hi_axiom(_size != 0);
        std::move(_cache.begin() + 1, _cache.begin() + _size, _cache.begin());
        --_size;

        if (_it != std:default_sentinel) {
            _cache[_size++] = std::move(*_it++);
        }

        return *this;
    }

private:
    iterator _it = {};
    size_t _size = 0;
    std::array<value_type, max_size> _cache = {};
};

template<size_t LookaheadCount, typename Iterator>
auto make_lookahead_iterator(Iterator &&it) noexcept
{
    return lookahead_iterator<LookaheadCount, std::decay_t<Iterator>>{hi_forward(it)};
}

}}

