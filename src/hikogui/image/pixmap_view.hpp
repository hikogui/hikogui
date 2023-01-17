

namespace hi {
inline namespace v1 {

template<typename T>
class pixmap_view {
public:
    using value_type = T;
    using nc_value_type = std::remove_const_t<value_type>;
    using size_type = size_t;

    ~pixmap_view() = default;
    constexpr pixmap_view(pixmap_view const &) noexcept = default;
    constexpr pixmap_view(pixmap_view &&) noexcept = default;
    constexpr pixmap_view &operator=(pixmap_view const &) noexcept = default;
    constexpr pixmap_view &operator=(pixmap_view &&) noexcept = default;
    [[nodiscard]] constexpr pixmap_view() noexcept = default;

    [[nodiscard]] constexpr pixmap_view(value_type *data, size_type width, size_type height, size_type stride) noexcept :
        _data(data), _width(width), _height(height), _stride(stride) {}

    [[nodiscard]] constexpr pixmap_view(value_type *data, size_type width, size_type height) noexcept :
        pixmap_view(data, width, height, width) {}

    [[nodiscard]] constexpr pixmap_view(pixmap<nc_value_type> const &map) noexcept :
        pixmap_view(map.data(), map.width(), map.height(), map.stride()) {}

    [[nodiscard]] constexpr size_type width() const noexcept
    {
        return _width;
    }

    [[nodiscard]] constexpr size_type height() const noexcept
    {
        return _height;
    }

    [[nodiscard]] constexpr size_type stride() const noexcept
    {
        return _stride;
    }

    [[nodiscard]] constexpr value_type *data() noexcept
    {
        return _data;
    }

    [[nodiscard]] constexpr value_type const *data() const noexcept
    {
        return _data;
    }

    constexpr value_type &operator()(size_type x, size_type y) noexcept
    {
        hi_axiom(x < width);
        hi_axiom(y < height);
        return _data[y * _stride + x];
    }

    constexpr value_type const &operator()(size_type x, size_type y) const noexcept
    {
        hi_axiom(x < width);
        hi_axiom(y < height);
        return _data[y * _stride + x];
    }

    [[nodiscard]] constexpr std::span<value_type> operator[](size_type y) noexcept
    {
        hi_axiom(y < height);
        return {_data + y * _stride, width};
    }

    [[nodiscard]] constexpr std::span<value_type const> operator[](size_type y) const noexcept
    {
        hi_axiom(y < height);
        return {_data + y * _stride, width};
    }

    [[nodiscard]] constexpr pixmap_view subimage(size_type x, size_type y, size_type width, size_type height) noexcept
    {
        return {_data + y * _stride + x, width, height, _stride};
    }

    [[nodiscard]] constexpr pixmap_view<value_type const> subimage(size_type x, size_type y, size_type width, size_type height) const noexcept
    {
        return {_data + y * _stride + x, width, height, _stride};
    }

private:
    value_type *_data = nullptr;
    size_type _width = 0;
    size_type _height = 0;
    size_type _stride = 0;
};


}}

