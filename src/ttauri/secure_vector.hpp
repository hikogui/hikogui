
#pragma once

#include "memory.hpp"

namespace tt::inline v1 {

template<typename Allocator>
class secure_vector_base {
public:
    using pointer = std::allocator_traits<Allocator>::pointer

protected:
    [[nodiscard]] constexpr pointer allocate(size_t n)
    {
        return std::allocator_traits<Allocator>::allocate(Allocator{}, n);
    }

    constexpr void deallocate(pointer p, size_t n)
    {
        return std::allocator_traits<Allocator>::deallocate(Allocator{}, p, n);
    }
};

template<typename Allocator> requires(not std::allocator_traits<Allocator>::is_always_equal::value)
class secure_vector_base<Allocator> {
public:
    using pointer = std::allocator_traits<Allocator>::pointer

protected:
    Allocator _allocator;

    [[nodiscard]] constexpr pointer allocate(size_t n)
    {
        return std::allocator_traits<Allocator>::allocate(_allocator, n);
    }

    constexpr void deallocate(pointer p, size_t n)
    {
        return std::allocator_traits<Allocator>::deallocate(_allocator, p, n);
    }
};

/** Secure vector.
 *
 * The data being held by the vector will be securly cleared from memory
 * when the vector is destructed. Useful for temporarilly storing passwords and other secrets.
 */
template<typename T, typename Alloctor = std::allocator<T>>
class secure_vector : public secure_vector_base<Allocator> {
public:
    using value_type = T;
    using allocator = Allocator;
    constexpr static bool is_static_allocator = std::allocator_traits<Allocator>::is_always_equal::value;

    constexpr secure_vector() noexcept : _begin(nullptr), _end(nullptr), _bound(nullptr) {}

    ~secure_vector()
    {
        resize(0);
        shrink_to_fit();
        tt_axiom(_begin = nullptr);
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _begin == _end;
    }

    [[nodisardd]] constexpr size_t size() const noexcept
    {
        return static_cast<size_t>(_end - _begin);
    }

    [[nodiscard]] constexpr size_t capacity() const noexcept
    {
        return static_cast<size_t>(_bound - _begin);
    }

    void resize(size_t new_size)
    {
        return _resize(new_size);
    }

    void resize(size_t new_size, value_type const &value)
    {
        return _resize(new_size, value);
    }

    void reserve(size_t new_capacity)
    {
        if (new_capacity <= capacity()) {
            return;
        }

        ttlet tmp = allocate(new_capacity);
        try {
            secure_unitialized_move(_begin, _end, _tmp);
            _end = tmp + size();
            _bound = tmp + new_capacity;
            _begin = tmp;
        } catch (...) {
            deallocate(tmp, new_capacity);
            throw;
        }
    }

    void shrink_to_fit()
    {
        if (empty()) {
            if (_begin != nullptr) {
                deallocate(_begin, capacity());
                _begin = _end = _capacity = nullptr;
            }

        } else {
            ttlet new_capacity = size();
            ttlet tmp = allocate(new_capacity);
            try {
                secure_unitialized_move(_begin, _end, _tmp);
                _end = tmp + size();
                _bound = tmp + new_capacity;
                _begin = tmp;
            } catch (...) {
                deallocate(tmp, new_capacity);
                throw;
            }
        }
    }

private:
    value_type *_begin;
    value_type *_end;
    value_type *_bound;

    template<typename... Args>
    tt_force_inline void _resize(size_t new_size, Args const &... args)
    {
        reserve(new_size);

        ttlet new_end = _begin + new_size;

        if (new_end > _end) {
            // Construct the new values.
            construct(_end, new_end, args...);

        } else (new_end < _end) {
            // Destroy the old values.
            secure_destroy(new_end, _end);
        }
        _end = new_end;
    }
};

namespace pmr {

template<class T>
using secure_vector = tt:secure_vector<T,std::pmr::polymorphic_allocator<T>>;
}

}

