

#pragma once

#include <memory>

namespace tt {

template<typename Key, typename T>
struct hash_map_entry {
    using key_type = Key;
    using value_type = T;

    size_t hash;
    key_type key;
    value_type value;
};

template<typename Key, typename T, typename Allocator = std::allocator<hash_map_entry<Key,T>>>
class hash_map {
public:
    using key_type = Key;
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using allocator_type = Allocator;
    using reference = value_type &;
    using const_reference = value_type const &;
    using pointer = value_type *;
    using const_pointer = value_type const *;
    using node_type = hash_map_entry<Key,T>;
    constexpr static bool allocator_always_equal = std::allocator_traits<allocator_type>::is_always_equal::value;

    constexpr hash_map() noexcept requires (allocator_always_equal) : allocator() {}


    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept
    {
        return allocator;
    }

private:
    node_type *_nodes = nullptr;
    size_t _capacity = 0;
    size_t _size = 0;
    [[no_unique_address]] allocator_type allocator;
};

namespace pmr {

template<typename Key, typename T>
using hash_map = hash_map<Key, T, std::pmr::polymorphic_allocator<tt::hash_map_entry<Key, T>>>;

}
}

