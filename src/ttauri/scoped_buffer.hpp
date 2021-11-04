

#pragma once

#include <memory_resource>
#include <array>

namespace tt::inline v1::pmr {

/** A buffer with an attached memory allocator.
 */
template<size_t Size, typename T = std::byte>
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

} // namespace tt::inline v1::pmr
