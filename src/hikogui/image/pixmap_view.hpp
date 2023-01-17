// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility.hpp"
#include "../assert.hpp"
#include <cstddef>
#include <span>
#include <memory>

namespace hi { inline namespace v1 {
template<typename T, typename Allocator>
class pixmap;

template<typename T>
class pixmap_view {
public:
    using value_type = T;
    using reference = value_type&;
    using const_reference = value_type const&;
    using pointer = value_type *;
    using const_pointer = value_type const *;
    using row_type = std::span<value_type>;
    using const_row_type = std::span<value_type const>;
    using nc_value_type = std::remove_const_t<value_type>;
    using size_type = size_t;

    ~pixmap_view() = default;
    constexpr pixmap_view(pixmap_view const&) noexcept = default;
    constexpr pixmap_view(pixmap_view&&) noexcept = default;
    constexpr pixmap_view& operator=(pixmap_view const&) noexcept = default;
    constexpr pixmap_view& operator=(pixmap_view&&) noexcept = default;
    [[nodiscard]] constexpr pixmap_view() noexcept = default;

    [[nodiscard]] constexpr pixmap_view(value_type *data, size_type width, size_type height, size_type stride) noexcept :
        _data(data), _width(width), _height(height), _stride(stride)
    {
    }

    [[nodiscard]] constexpr pixmap_view(value_type *data, size_type width, size_type height) noexcept :
        pixmap_view(data, width, height, width)
    {
    }

    template<std::same_as<std::remove_const_t<value_type>> O, typename Allocator>
    [[nodiscard]] constexpr pixmap_view(pixmap<O, Allocator> const& other) noexcept :
        pixmap_view(other.data(), other.width(), other.height())
    {
    }

    template<std::same_as<std::remove_const_t<value_type>> O, typename Allocator>
    [[nodiscard]] constexpr pixmap_view(pixmap<O, Allocator>& other) noexcept :
        pixmap_view(other.data(), other.width(), other.height())
    {
    }

    template<std::same_as<std::remove_const_t<value_type>> O, typename Allocator>
    [[nodiscard]] constexpr pixmap_view(pixmap<O, Allocator>&& other) = delete;

    [[nodiscard]] constexpr size_type empty() const noexcept
    {
        return _width == 0 and _height == 0;
    }

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

    [[nodiscard]] constexpr pointer data() noexcept
    {
        return _data;
    }

    [[nodiscard]] constexpr const_pointer data() const noexcept
    {
        return _data;
    }

    constexpr reference operator()(size_type x, size_type y) noexcept
    {
        hi_axiom(x < _width);
        hi_axiom(y < _height);
        return _data[y * _stride + x];
    }

    constexpr const_reference operator()(size_type x, size_type y) const noexcept
    {
        hi_axiom(x < _width);
        hi_axiom(y < _height);
        return _data[y * _stride + x];
    }

    [[nodiscard]] constexpr row_type operator[](size_type y) noexcept
    {
        hi_axiom(y < _height);
        return {_data + y * _stride, _width};
    }

    [[nodiscard]] constexpr const_row_type operator[](size_type y) const noexcept
    {
        hi_axiom(y < _height);
        return {_data + y * _stride, _width};
    }

    [[nodiscard]] constexpr pixmap_view subimage(size_type x, size_type y, size_type new_width, size_type new_height) noexcept
    {
        return {_data + y * _stride + x, new_width, new_height, _stride};
    }

    [[nodiscard]] constexpr pixmap_view<value_type const>
    subimage(size_type x, size_type y, size_type new_width, size_type new_height) const noexcept
    {
        return {_data + y * _stride + x, new_width, new_height, _stride};
    }

    constexpr friend void copy(pixmap_view src, pixmap_view<std::remove_const_t<value_type>> dst) noexcept
    {
        hi_axiom(src.width() == dst.width());
        hi_axiom(src.height() == dst.height());

        if (src.width() == src.stride() and dst.width() == dst.stride()) {
            std::copy(src.data(), src.data() + src.width() * src.height(), dst.data());
        } else {
            for (auto y = 0_uz; y != src.height(); ++y) {
                hilet src_line = src[y];
                hilet dst_line = dst[y];
                std::copy(src_line.begin(), src_line.end(), dst_line.begin());
            }
        }
    }

    constexpr friend void fill(pixmap_view dst, value_type value = value_type{}) noexcept
    {
        if (dst._width == dst._stride) {
            std::fill_n(dst._data, dst._width, dst._height, value);
        } else {
            for (auto y = 0_uz; y != dst._height; ++y) {
                hilet line = dst[y];
                std::fill(line.begin(), line.end(), value);
            }
        }
    }

private:
    value_type *_data = nullptr;
    size_type _width = 0;
    size_type _height = 0;
    size_type _stride = 0;
};

template<typename T, typename Allocator>
pixmap_view(pixmap<T, Allocator> const& other) -> pixmap_view<T const>;

template<typename T, typename Allocator>
pixmap_view(pixmap<T, Allocator>& other) -> pixmap_view<T>;

}} // namespace hi::v1
