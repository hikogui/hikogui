
#pragma once

#include "../macros.hpp"
#include <vector>

hi_export_module(hikogui.container : fifo)


hi_export namespace hi::inline v1 {

template<typename T, typename Allocator = std::allocator<T>>
class fifo {
public:
    using value_type = T;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = value_type const&;
    using pointer = std::allocator_traits<Allocator>::pointer;
    using const_pointer = std::allocator_traits<Allocator>::const_pointer;
   
    constexpr fifo() noexcept = default;

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _head == _tail;
    }

    [[nodiscard]] constexpr size_type size() const noexcept
    {
        if (_head => _tail) {
            return std::distance(_tail, _head);
        } else {
            return std::distance(_begin, _head) + std::distance(_tail, _end);
        }
    }

    [[nodiscard]] constexpr size_type max_size() const noexcept
    {
        return std::allocator_traits<allocator_type>::max_size();
    }

    [[nodiscard]] constexpr size_type capacity() const noexcept
    {
        return std::distance(_begin, _end);
    }

    constexpr reserve(size_t new_size)
    {
        if (new_size > max_size()) {
            return std::length_error("Reservation too large");
        }

        auto const old_size = capacity();
        if (new_size <= old_size) {
            return;
        }

        // Use the growth factor of 1.5 which is preferred as it
        // is more likely to reuse previous allocations.
        auto grow_size = old_size;
        while (grow_size < new_size) {
            grow_size += grow_size >> 1;
        }

        reallocate(grow_size);
    }

    constexpr shrink_to_fit()
    {
        reallocate(size());
    }

    constexpr clear()
    {
        if (_head >= _tail) {
            std::destroy(_tail, _head);
        } else {
            std::destroy(_tail, _end);
            std::destroy(_begin, _head);
        } 
        _head = _tail = _begin;
    }

    [[nodiscard]] constexpr const_reference front() const
    {
        assert(_head != _tail);
        return *_tail;
    }

    [[nodiscard]] constexpr reference front()
    {
        assert(_head != _tail);
        return *_tail;
    }

    [[nodiscard]] constexpr const_reference back() const
    {
        assert(_head != _tail);
        return *(_head - 1);
    }

    [[nodiscard]] constexpr reference back()
    {
        assert(_head != _tail);
        return *(_head - 1);
    }

    [[nodiscard]] constexpr const_reference operator[](size_t i) const
    {
        assert(i < size());

        auto const wrap_size = std::distance(_tail, _end);
        if (i < wrap_size) {
            return _tail[i];
        }

        i -= wrap_size;
        return _begin[i];
    }

    [[nodiscard]] constexpr reference operator[](size_t i)
    {
        assert(i < size());

        auto const wrap_size = std::distance(_tail, _end);
        if (i < wrap_size) {
            return _tail[i];
        }

        i -= wrap_size;
        return _begin[i];
    }

    constexpr void pop_front()
    {
        assert(_head != _tail);

        std::destroy_at(_tail);
        if (++_tail == _end) {
            _tail = _begin;
        }
    }

    constexpr void pop_back()
    {
        assert(_head != _tail);

        if (_head-- == _begin) {
            _head = _end - 1;
        }
        std::destroy_at(_head);
    }

    template<typename... Args>
    constexpr reference emplace_front(Args&&... args)
    {
        reserve(size() + 1);
        assert(_tail != _end);

        if (_tail-- == _begin) {
            _tail = _end - 1;
        }

        return *std::construct_at(_tail, std::forward<Args>(args)...);
    }

    constexpr void push_front(value_type const& value)
    {
        emplace_front(value);
    }

    constexpr void push_front(value_type&& value)
    {
        emplace_front(std::move(value));
    }

    template<typename... Args>
    constexpr value_type& emplace_back(Args&&... args)
    {
        reserve(size() + 1);
        assert(_head != _end);

        auto *ptr = std::construct_at(_head, std::forward<Args>(value)...);
        if (++_head == _end) {
            _head = _begin;
        }
        return *ptr;
    }

    constexpr void push_back(value_type const& value)
    {
        emplace_back(value);
    }

    constexpr void push_back(value_type&& value)
    {
        emplace_back(std::move(value));
    }

private:
    allocator_type _allocator;
    pointer _begin = nullptr;
    pointer _end = nullptr;
    pointer _head = nullptr;
    pointer _tail = nullptr;

    /** Reallocate the data in the fifo.
     *
     * This has the same exception guarantees as std::vector:
     *  - If allocation fails, then the fifo is not modified.
     *  - If the move of objects fails, then the objects in the fifo
     *    may be in a valid but unspecified state.
     */
    void reallocate(size_t new_size)
    {
        assert(new_size <= size());

        auto const *new_begin = std::allocator_traits<allocator_type>::allocate(_allocator, new_size);
        auto const *new_end = new_begin + new_size;
        auto const *new_tail = new_begin;
        auto const *new_head = new_tail + size();

        try {
            if (_head >= _tail) {
                std::uninitialized_move(_tail, _head, new_begin);

            } else {
                std::uninitialized_move(_tail, _end, new_begin);

                auto const partial_size = std::distance(_tail, _end);
                try {
                    std::uninitialized_move(_begin, _head, new_begin + partial_size);
                } catch (...) {
                    std::destroy_n(_begin, partial_size);
                    throw;
                }
            }
        } catch (...) {
            std::allocator_traits<allocator_type>::deallocate(_allocator, new_begin, std::distance(new_begin, new_end));
            throw;
        }

        // Destroy the moved from objects.
        if (_head >= _tail) {
            std::destroy(_tail, _head);
        } else {
            std::destroy(_tail, _end);
            std::destroy(_begin, _head);
        } 

        auto const *old_begin = std::exchange(_begin, new_begin);
        auto const *old_end = std::exchange(_end, new_end);
        _tail = new_tail;
        _head = new_head;
        std::allocator_traits<allocator_type>::deallocate(_allocator, old_begin, std::distance(old_begin, old_end));
    }
};

namespace mpr {

template<typename T>
using fifo = ::hi::fifo<T, std::pmr_polymorpic_allocator<T>>;

}

}

