// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


#pragma once

#include <memory_resource>
#include <array>

namespace hi::inline v1::pmr {

/** A buffer with an attached memory allocator.
 */
template<std::size_t Size, typename T = std::byte>
class scoped_buffer {
public:
    using value_type = T;
    using allocator_type = std::pmr::polymorphic_allocator<value_type>;

    constexpr scoped_buffer() noexcept : _buffer(), _mbr(_buffer.data(), _buffer.size()), _pa(&_mbr) {}

    constexpr allocator_type &allocator() noexcept
    {
        return _pa;
    }

private:
    std::array<value_type, Size> _buffer;
    std::pmr::monotonic_buffer_resource _mbr;
    allocator_type _pa;
};

} // namespace hi::inline v1::pmr
