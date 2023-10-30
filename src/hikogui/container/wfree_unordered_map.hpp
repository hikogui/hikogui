// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <atomic>
#include <array>
#include <optional>
#include <vector>
#include <typeinfo>
#include <typeindex>

hi_export_module(hikogui.container.wfree_unordered_map);

hi_export namespace hi::inline v1 {

template<typename K, typename V>
struct wfree_unordered_map_item {
    /*! The value.
     * It comes first because it is of unknown size
     */
    V value;

    /*! Hash for quick comparison and for state.
     * Special values:
     *  * 0 = Empty
     *  * 1 = Busy
     *  * 2 = Tombstone
     *
     * Natural hash values 0, 1, 2 must be mapped to 3, 4, 5.
     */
    std::atomic<std::size_t> hash;
    K key;

    constexpr wfree_unordered_map_item() noexcept requires(std::is_same_v<K, std::type_index>) :
        value(), hash(0), key(std::type_index(typeid(void)))
    {
    }

    constexpr wfree_unordered_map_item() noexcept requires(not std::is_same_v<K, std::type_index>) : value(), hash(0), key() {}

    constexpr wfree_unordered_map_item(wfree_unordered_map_item const &) noexcept = delete;
    constexpr wfree_unordered_map_item(wfree_unordered_map_item &&) noexcept = delete;
    ~wfree_unordered_map_item() noexcept = default;
    constexpr wfree_unordered_map_item &operator=(wfree_unordered_map_item const &) noexcept = delete;
    constexpr wfree_unordered_map_item &operator=(wfree_unordered_map_item &&) noexcept = delete;
};

/*! Unordered map with wait-free insert, get and erase.
 *
 * This class can be instantiated as a global variable without
 * needing initialization.
 */
template<typename K, typename V, std::size_t MAX_NR_ITEMS>
class wfree_unordered_map {
public:
    using key_type = K;
    using mapped_type = V;

private:
    constexpr static std::size_t CAPACITY = MAX_NR_ITEMS * 2;

    std::array<wfree_unordered_map_item<K, V>, CAPACITY> items = {};

public:
    constexpr wfree_unordered_map() noexcept = default;
    constexpr wfree_unordered_map(wfree_unordered_map const &) noexcept = delete;
    constexpr wfree_unordered_map(wfree_unordered_map &&) noexcept = delete;
    ~wfree_unordered_map() noexcept = default;
    constexpr wfree_unordered_map &operator=(wfree_unordered_map const &) noexcept = delete;
    constexpr wfree_unordered_map &operator=(wfree_unordered_map &&) noexcept = delete;

    static std::size_t make_hash(K const &key) noexcept
    {
        hilet hash = std::hash<K>{}(key);
        return hash >= 3 ? hash : hash + 3;
    }

    void insert(K key, V value) noexcept
    {
        hilet hash = make_hash(key);

        auto index = hash % CAPACITY;
        while (true) {
            auto &item = items[index];

            // First look for an empty (0) item, highly likely when doing insert.
            std::size_t item_hash = 0;
            if (item.hash.compare_exchange_strong(item_hash, 1, std::memory_order::acquire)) {
                // Success, we found an empty entry, we marked it as busy (1).
                item.key = std::move(key);
                item.value = value;
                item.hash.store(hash, std::memory_order::release);
                return;

            } else if (item_hash == hash && key == item.key) {
                // Key was already in map, replace the value.
                item.value = value;
                return;

            } else {
                // Either this item was already in use by another key, or
                // Another thread was ahead of us with claiming this item (hopefully the other
                // thread doesn't insert the same key).
                // Even though compare_exchange was used here, this algorithm is wait-free since all threads
                // including the one here are making normal progress.
                index = (index + 1) % CAPACITY;
            }
        }
    }

    std::vector<K> keys() const noexcept
    {
        std::vector<K> r;
        // XXX - with counting items, we could reserve capacity.

        for (hilet &item : items) {
            if (item.hash >= 3) {
                r.push_back(item.key);
            }
        }
        return r;
    }

    V &operator[](K const &key) noexcept
    {
        hilet hash = make_hash(key);

        auto index = hash % CAPACITY;
        while (true) {
            auto &item = items[index];

            // First look for an empty (0) item, highly likely when doing insert.
            std::size_t item_hash = 0;
            if (item.hash.compare_exchange_strong(item_hash, 1, std::memory_order::acquire)) {
                // Success, we found an empty entry, we marked it as busy (1).
                item.key = std::move(key);
                item.hash.store(hash, std::memory_order::release);

                if constexpr (std::is_default_constructible_v<V>) {
                    item.value = {};
                } else {
                    // Make sure we default initialize the memory.
                    std::memset(&(item.value), 0, sizeof(item.value));
                }
                return item.value;

            } else if (item_hash == hash && key == item.key) {
                // Key was already in map, replace the value.
                return item.value;

            } else {
                // Either this item was already in use by another key, or
                // Another thread was ahead of us with claiming this item (hopefully the other
                // thread doesn't insert the same key).
                // Even though compare_exchange was used here, this algorithm is wait-free since all threads
                // including the one here are making normal progress.
                index = (index + 1) % CAPACITY;
            }
        }
    }

    std::optional<V> get(K const &key) const noexcept
    {
        hilet hash = make_hash(key);

        auto index = hash % CAPACITY;
        while (true) {
            auto &item = items[index];

            auto item_hash = item.hash.load(std::memory_order::acquire);

            if (item_hash == hash && key == item.key) {
                // Found key
                return {item.value};

            } else if (item_hash == 0) {
                // Item is empty.
                return {};

            } else {
                index = (index + 1) % CAPACITY;
            }
        }
    }

    V get(K const &key, V const &default_value) const noexcept
    {
        if (hilet optional_value = get(key)) {
            return *optional_value;
        } else {
            return default_value;
        }
    }

    std::optional<V> erase(K const &key) noexcept
    {
        hilet hash = make_hash(key);

        auto index = hash % CAPACITY;
        while (true) {
            auto &item = items[index];
            auto item_hash = item.hash.load(std::memory_order::acquire);

            if (item_hash == hash && key == item.key) {
                // Set tombstone. Don't actually delete the key or value.
                item.hash.store(1, std::memory_order::release);
                return {item.value};

            } else if (item_hash == 0) {
                // Item is empty.
                return {};

            } else {
                index = (index + 1) % CAPACITY;
            }
        }
    }
};

} // namespace hi::inline v1
