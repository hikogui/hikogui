// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility/module.hpp"
#include <memory>
#include <iterator>

namespace hi::inline v1 {
namespace detail {

struct hash_map_entry_h {
    std::size_t hash;
};

template<typename Key, typename Value>
struct hash_map_entry_hkv {
    std::size_t hash;
    Key key;
    Value value;
};

} // namespace detail

template<typename Key, typename T, typename Allocator>
class hash_map;

template<typename Key, typename Value>
class hash_map_entry {
public:
    using key_type = Key;
    using value_type = Value;

    constexpr hash_map_entry() noexcept : _h{0} {}
    constexpr hash_map_entry(hash_map_entry const&) noexcept = default;
    constexpr hash_map_entry(hash_map_entry&&) noexcept = default;
    constexpr hash_map_entry &operator=(hash_map_entry const&) noexcept = default;
    constexpr hash_map_entry &operator=(hash_map_entry&&) noexcept = default;
    constexpr ~hash_map_entry()
    {
        if (hash() != 0) {
            std::destroy_at(std::addressof(_hkv));
        }
    }

    std::size_t hash() const noexcept
    {
        return _h.hash;
    }

    key_type const &key() const noexcept
    {
        return _hkv.key;
    }

    value_type &value() noexcept
    {
        return _hkv.value;
    }

    value_type const &value() const noexcept
    {
        return _hkv.value;
    }

private:
    template<typename K, typename V>
    constexpr void set(std::size_t hash, K &&key, V &&value) noexcept
    {
        if (this->hash() != 0) {
            std::destroy_at(std::addressof(_hkv));
        }
        std::construct_at(std::addressof(_hkv), hash, std::forward<K>(key), std::forward<V>(value));
    }

    template<typename K>
    constexpr void set(std::size_t hash, K &&key) noexcept
    {
        if (this->hash() != 0) {
            std::destroy_at(std::addressof(_hkv));
        }
        std::construct_at(std::addressof(_hkv), hash, std::forward<K>(key), Value{});
    }

    union {
        detail::hash_map_entry_h _h;
        detail::hash_map_entry_hkv<key_type, value_type> _hkv;
    };

    template<typename, typename, typename>
    friend class hash_map;
};

template<typename Key, typename T, typename Allocator = std::allocator<hash_map_entry<Key, T>>>
class hash_map {
public:
    using key_type = Key;
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = ptrdiff_t;
    using allocator_type = Allocator;
    using reference = value_type &;
    using const_reference = value_type const &;
    using pointer = value_type *;
    using const_pointer = value_type const *;
    using node_type = hash_map_entry<Key, T>;
    constexpr static bool allocator_always_equal = std::allocator_traits<allocator_type>::is_always_equal::value;

    class iterator {
    public:
        constexpr iterator(node_type *node, bool used) noexcept : node(node), used(used) {}

        [[nodiscard]] constexpr node_type &operator*() const noexcept
        {
            return *node;
        }

        [[nodiscard]] constexpr node_type *operator->() const noexcept
        {
            return node;
        }

        [[nodiscard]] constexpr friend bool operator==(iterator const &lhs, std::default_sentinel_t) noexcept
        {
            return not lhs.used;
        }

    private:
        bool used;
        node_type *node;
    };

    class const_iterator {
    public:
        constexpr const_iterator(node_type *node) noexcept : node(node) {}

        [[nodiscard]] constexpr node_type const &operator*() const noexcept
        {
            return *node;
        }

        [[nodiscard]] constexpr node_type const *operator->() const noexcept
        {
            return node;
        }

        [[nodiscard]] constexpr friend bool operator==(iterator const &lhs, std::default_sentinel_t) noexcept
        {
            return node->hash;
        }

    private:
        node_type *node;
    };

    hash_map(hash_map const &) = delete;
    hash_map(hash_map &&) = delete;
    hash_map &operator=(hash_map const &) = delete;
    hash_map &operator=(hash_map &&) = delete;

    constexpr hash_map() noexcept requires(allocator_always_equal) : _nodes(nullptr), _capacity(0), _size(0), allocator()
    {
        reserve(initial_capacity);
    }

    constexpr ~hash_map()
    {
        hi_assert_not_null(_nodes);
        hi_axiom(_capacity != 0);

        std::destroy_n(_nodes, _capacity);
        std::allocator_traits<allocator_type>::deallocate(allocator, _nodes, _capacity);
    }

    [[nodiscard]] constexpr allocator_type get_allocator() const noexcept
    {
        return allocator;
    }

    hi_no_inline constexpr void reserve(std::size_t new_capacity) noexcept
    {
        if (new_capacity > _capacity) {
            auto *new_nodes = std::allocator_traits<allocator_type>::allocate(allocator, new_capacity);

            move_nodes(_nodes, _capacity, new_nodes, new_capacity);

            if (_nodes != nullptr) {
                std::allocator_traits<allocator_type>::deallocate(allocator, _nodes, _capacity);
            }
            _nodes = new_nodes;
            _capacity = new_capacity;
        }
    }

    [[nodiscard]] constexpr const_iterator find(key_type const &key) const noexcept
    {
        hi_assert_not_null(_nodes);
        hi_axiom(_capacity != 0);

        auto hash = std::hash<key_type>{}(key);
        hash = hash == 0 ? 1 : hash;

        auto hash_plus_count = hash;
        while (true) {
            // _capacities are selected for their ability to avalanche bad hash values.
            auto node = _nodes + hash_plus_count % _capacity;
            if (node->hash() == 0 or (node->hash() == hash and node->key() == key)) {
                return const_iterator{node};
            }

            ++hash_plus_count;
        }
    }

    template<typename K>
    [[nodiscard]] constexpr iterator find_or_create(K &&key) noexcept
    {
        hi_assert_not_null(_nodes);
        hi_axiom(_capacity != 0);

        auto hash = std::hash<key_type>{}(key);
        hash = hash == 0 ? 1 : hash;

        auto hash_plus_count = hash;
        while (true) {
            // _capacities are selected for their ability to avalanche bad hash values.
            auto node = _nodes + hash_plus_count % _capacity;
            hi_axiom_not_null(node);
            if (node->hash() == hash and node->key() == key) {
                return iterator{node, true};

            } else if (node->hash() == 0) {
                return or_create(*node, hash, std::forward<K>(key));
            }

            ++hash_plus_count;
        }
    }

    template<typename K>
    [[nodiscard]] constexpr value_type &operator[](K &&key) noexcept
    {
        return find_or_create(std::forward<K>(key))->value();
    }

    [[nodiscard]] constexpr std::default_sentinel_t end() const noexcept
    {
        return {};
    }

    [[nodiscard]] constexpr std::default_sentinel_t cend() const noexcept
    {
        return {};
    }

private:
    static constexpr std::size_t initial_capacity = 307;

    node_type *_nodes = nullptr;
    std::size_t _capacity = 0;
    std::size_t _size = 0;
    [[no_unique_address]] allocator_type allocator;

    template<typename K>
    hi_no_inline constexpr iterator or_create(node_type &node, std::size_t hash, K &&key) noexcept
    {
        grow_by(1);
        node.set(hash, std::forward<K>(key));
        return iterator{&node, false};
    }

    /** move the nodes from one allocation to another.
     *
     * This function will std::move() the keys and values from one allocation to another.
     * Also all the from nodes will be destructed, and all the to nodes will be constructed.
     */
    hi_no_inline constexpr void move_nodes(node_type *src, std::size_t src_size, node_type *dst, std::size_t dst_size) noexcept
    {
        hi_axiom_not_null(dst);
        hilet dst_ = std::uninitialized_value_construct_n(dst, dst_size);
        hi_axiom_not_null(dst_);

        auto const * const src_last = src + src_size;
        for (auto src_it = src; src_it != src_last; ++src_it) {
            hi_axiom_not_null(src_it);
            hilet hash = src_it->hash();

            if (hash != 0) {
                auto hash_plus_count = hash;
                while (true) {
                    hilet dst_it = dst_ + hash_plus_count % dst_size;
                    hi_axiom_not_null(dst_it);

                    if (dst_it->hash() == 0) {
                        dst_it->set(hash, std::move(src_it->_hkv.key), std::move(src_it->_hkv.value));
                        break;
                    }

                    ++hash_plus_count;
                }
            }
        }

        std::destroy_n(src, src_size);
    }

    void grow_by(std::size_t nr_entries) noexcept
    {
        hi_assert_not_null(_nodes);
        hi_axiom(_capacity != 0);

        _size += nr_entries;

        // 0.75 fill ratio.
        hilet max_size = _capacity - (_capacity >> 2);
        if (_size > max_size) {
            // Using a growth factor of about 1.5 will allow reallocation in the holes left behind multiple
            // consecutive grows. Make the new capacity odd, to increase chance for good avalanching.
            auto new_capacity = (_capacity + (_capacity >> 1) + nr_entries) | 1;
            reserve(new_capacity);
        }
    }
};

namespace pmr {

template<typename Key, typename T>
using hash_map = hi::hash_map<Key, T, std::pmr::polymorphic_allocator<hi::hash_map_entry<Key, T>>>;

}
} // namespace hi::inline v1
