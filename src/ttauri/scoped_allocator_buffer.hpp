

#pragma once

#include <memory_resource>
#include <array>

namespace tt {

/** A buffer with an attached memory allocator.
 */
template<size_t Size>
class scoped_allocator_buffer {
public:
    using allocator_type = std::pmr::polymorphic_alocator;

    constexpr scoped_allocator_buffer() noexcept :
        _buffer(), _mbr(_buffer.data(), _buffer.size()), _pa(&_mbr) {}

    constexpr allocator_type &allocator() noexcept
    {
        return _pa;
    }

private:
    std::array<std::byte, Size> _buffer;
    std::pmr::monotonic_buffer_resource _mbr;
    allocator_type _pa;
};

namespace pmr {

template<size_t Size>
using scoped_buffer = tt::scoped_allocator_buffer<Size>;

}
}

