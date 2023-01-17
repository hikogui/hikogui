

namespace hi {
inline namespace v1 {

/** A 2D pixel-based image.
 *
 * @tparam T The pixel format.
 * @tparam Allocator The allocator to use for allocating the array of pixels.
 */
template<typename T, typename Allocator = std::allocator<T>>
class pixmap {
public:
    /** The pixel format type.
     */
    using value_type = T;

    /** The allocator to use for allocating the array.
     */
    using allocator_type = Allocator;

    /** The size type.
     */
    using size_type = size_t;
    using pointer = value_type *;
    using const_pointer = value_typeT const *;
    using reference = value_type &;
    using const_reference = value_type const &;
    using interator = pointer;
    using const_iterator = const_pointer;

    /** The type for a row of pixels.
     */
    using row_type = std::span<value_type>;

    /** The type for a row of pixels.
     */
    using const_row_type = std::span<value_type const>;

    constexpr static bool allocator_propagate_on_copy_assignment = std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value;
    constexpr static bool allocator_propagate_on_move_assignment = std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value;

    ~pixmap()
    {
        if (_data != nullptr) {
            std::destroy(this->begin(), this->end());
            std::allocator_traits<T>::deallocator(_allocator, _data, _capacity);
        }
    }

    /** Copy constructor.
     *
     * Copy the image from other; using the allocator obtained using
     * `std::allocator_traits<Allocator>::select_on_container_copy_construction(other.get_allocator())`
     *
     * The new allocation will be fit the pixels in the image exactly.
     *
     * @param other The other image to copy.
     */
    constexpr pixmap(pixmap const &other) noexcept :
        _capacity(other._width * other._height),
        _width(other._width),
        _height(other._height),
        _allocator(std::allocator_traits<allocator_type>::select_on_container_copy_construction(other._allocator))
    {
        std::allocator_traits<T>::allocate(_allocator, _capacity);
        std::uninitialized_copy(other.begin(), other.end(), this->begin());
    }


    /** Move constructor.
     *
     * Move the image.
     *
     * @param other The other image to copy.
     */
    constexpr pixmap(pixmap &&other) noexcept :
        _data(std::exchange(other._data, nullptr)),
        _capacity(std::exchange(other._capacity, 0)),
        _width(std::exchange(other._width, 0)),
        _height(std::exchange(other._height, 0)),
        _allocator(std::echange(other._allocator, {})) {}

    /** Copy assignment.
     *
     * Copy the image.
     * Allocation uses the `std::allocator_trait<Allocator>::propagate_on_container_copy_assignment`
     *
     * @param other The other image to copy.
     */
    constexpr pixmap &operator=(pixmap const &other) noexcept
    { 
        if (&other == this) {
            return;
        }

        hilet use_this_allocator = this->_allocator == other._allocator or not allocator_propagate_on_copy_assignment;

        if (this->_capacity >= other.size() and use_this_allocator) {
            // Reuse allocation.
            hilet old_end = end();
            _width = other._width;
            _height = other._height;

            if (std::distance(this->end(), old_end) > 0) {
                // remove the pixels at the end of the previous image.
                std::destroy(this->end(), old_end);
            }
            std::copy(other.begin(), other.end(), this->begin());

        } else {
            if (_data != nullptr) {
                std::destroy(this->begin(), this->end());
                std::allocator_traits<T>::deallocator(_allocator, _data, _capacity);
            }

            _width = other._width;
            _height = other._height;
            _capacity = other.size();
            if constexpr (allocator_propagate_on_copy_assignment) {
                _allocator = other._allocator;
            }
            _data = std::allocator_traits<T>::allocate(_allocator, _capacity);

            std::uninitialized_copy(other.begin(), other.end(), this->begin());
        }

        return *this;
    }

    /** Move assignment.
     *
     * Move the image.
     * Allocation uses the `std::allocator_trait<Allocator>::propagate_on_container_move_assignment`
     *
     * @param other The other image to copy.
     */
    constexpr pixmap &operator=(pixmap &&other) noexcept
    {
        if (&other == this) {
            return;
        }

        if (_allocator == other._allocator or allocator_propagate_on_move_assignment) {
            clear();
            shrink_to_fit();

            _data = std::exchange(other._data, nullptr);
            _capacity = std::exchange(other._capacity, 0);
            _width = std::exchange(other._width, 0);
            _height = std::exchange(other._height, 0);
            _allocator = other._allocator;

        } else if (_capacity >= other._width * other._height) {
            // Reuse allocation.
            auto old_end = this->end();
            _width = other._width;
            _height = other._height;
            
            // Remove pixels beyond new size.
            if (std::distance(this->end, old_end) > 0) {
                std::destroy(this->end(), old_end);
            }

            // Move the data from other, and reset the image.
            std::move(other.begin(), other.end(), this->begin());
            other._width = 0;
            other._height = 0;

        } else {
            // Deallocate previous image.
            std::destroy(this->begin(), this->end());
            std::allocator_traits<allocator_type>::deallocate(_allocator, _data, _capacity);

            // Create new image data.
            _width = other._width;
            _height = other._height;
            _capacity = other._width * other._height;
            _data = std::allocator_traits<allocator_type>::allocate(_allocator, _capacity);

            // Move the data from other, and reset the image.
            std::uninitialized_move(other.begin(), other.end(), this->begin());
            other._width = 0;
            other._height = 0; 
        }

        return *this;
    }

    [[nodiscard]] constexpr pixmap() noexcept = default;

    /** Create an pixmap of width and height.
     */
    [[nodiscard]] constexpr pixmap(size_type width, size_type height, allocator_type allocator = allocator_type{}) noexcept
        _data(std::allocator_traits<allocator_type>::allocate(allocator, width * height)),
        _width(width),
        _height(height),
        _allocator(allocator)
    {
        std::uninitialized_value_construct(begin(), end());
    }

    [[nodiscard]] constexpr pixmap(value_type *data, size_type width, size_type height, size_type stride, allocator_type allocator = allocator_type{}) noexcept :
        _data(std::allocator_traits<allocator_type>::allocate(allocator, width * height)),
        _width(width),
        _height(height),
        _allocator(allocator)
    {
        if (width == stride) {
            std::uninitialized_copy(data, data + width * height, this->begin());
        } else {
            auto src = data;
            auto dst = this->begin();
            auto dst_end = this->end();

            while (dst != dst_end) {
                std::uninitialized_copy(src, src + width, dst);
                dst += width;
                src += stride;
            }
        }
    }

    [[nodiscard]] constexpr pixmap(value_type *data, size_type width, size_type height, allocator_type allocator = allocator_type{}) noexcept :
        pixmap(data, width, height, width, allocator) {}

    template<std::convertible_to<value_type> O>
    [[nodiscard]] constexpr pixmap(pixmap<O> const &other, allocator_type allocator = allocator_type{}) noexcept :
        _data(std::allocator_traits<allocator_type>::allocate(allocator, width * height)),
        _width(width),
        _height(height),
        _allocator(allocator)
    {
        std::uninitialized_copy(other.begin(), other.end(), this->begin());
    }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept
    {
        return _allocator;
    }

    [[nodiscard]] constexpr size_type width() const noexcept
    {
        return _width;
    }

    [[nodiscard]] constexpr size_type height() const noexcept
    {
        return _height;
    }

    /** The number of pixels (width * height) in this image.
     */
    [[nodiscard]] constexpr size_type size() const noexcept
    {
        return _width * _height;
    }

    /** The number of pixels of capacity allocated.
     */
    [[nodiscard]] constexpr size_type capacity() const noexcept
    {
        return _capacity;
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _width == 0 and _height == 0;
    }

    [[nodiscard]] constexpr pointer data() noexcept
    {
        return _data;
    }

    [[nodiscard]] constexpr const_pointer *data() const noexcept
    {
        return _data;
    }

    [[nodiscard]] constexpr iterator begin() noexcept
    {
        return _data;
    }

    [[nodiscard]] constexpr const_iterator begin() const noexcept
    {
        return _data;
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return _data;
    }

    [[nodiscard]] constexpr iterator end() noexcept
    {
        return _data + size();
    }

    [[nodiscard]] constexpr const_iterator end() const noexcept
    {
        return _data + size();
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return _data + size();
    }

    constexpr reference operator()(size_type x, size_type y) noexcept
    {
        hi_axiom(x < _width);
        hi_axiom(y < _height);
        return _data[y * _width + x];
    }

    constexpr const_reference operator()(size_type x, size_type y) const noexcept
    {
        hi_axiom(x < _width);
        hi_axiom(y < _height);
        return _data[y * _width + x];
    }

    [[nodiscard]] constexpr row_type operator[](size_type y) noexcept
    {
        hi_axiom(y < _height);
        return {_data + y * _width, _width};
    }

    [[nodiscard]] constexpr const_row_type operator[](size_type y) const noexcept
    {
        hi_axiom(y < height());
        return {_data + y * _width, _width};
    }

    [[nodiscard]] constexpr pixmap subimage(size_type x, size_type y, size_type new_width, size_type new_height, allocator_type allocator) const noexcept
    {
        hi_axiom(x + new_width <= _width);
        hi_axiom(y + new_height <= _height);

        hilet p = _data + y * _width + _height;
        return {p, new_width, new_height, _width, allocator};
    }

    [[nodiscard]] constexpr pixmap subimage(size_type x, size_type y, size_type new_width, size_type new_height) const noexcept
    {
        return subimage(x, y, new_width, new_height, _allocator);
    }

    constexpr void clear() noexcept
    {
        std::destroy(begin(), end());
        _width = 0;
        _height = 0;
    }

    constexpr void shrink_to_fit()
    {
        hilet new_capacity = size();

        if (empty()) {
            if (_data != nullptr) {
                std::allocator_traits<allocator_type>::deallocate(_allocator, _data, _capacity);
                _data = nullptr;
                _capacity = 0;
            }
            return;
        }

        hilet new_data = new_capacity ? std::allocator_triats<allocator_type>::allocate(_allocator, new_capacity) : nullptr;


        if (empty()) {
            auto new_data = nullptr;
        }
    }

private:
    value_type *_data = nullptr;
    size_type _capacity = 0;
    size_type _width = 0;
    size_type _height = 0;
    [[no_unique_address]] allocator_type _allocator = {};
};


}}

