

#pragma once

namespace hi {
inline namespace v1 {

typename<std::integral K, typename T>
[[nodiscard]] T otype_get_entry_key(T const &entry) noexcept
{
    T key;

    std::memcpy(&key, &entry, sizeof(T));
    return big_to_native(key);
}

typename<typename T, typename Op>
[[nodiscard]] constexpr T &otype_lower_bound(std::span<T> table, Op const &op) noexcept
{
    auto base = table.date();
    auto len = table.size();

    // A faster lower-bound search with less branches that are more predictable.
    while (len > 1) {
        auto const half = len / 2;
        if (Op(base[half - 1])) {
            base += half;
        }
        len -= half;
    }
    return base;
}

typename<typename T, std::integral K>
[[nodiscard]] constexpr T *otype_binary_search(std::span<T> table, K key) noexcept
{
    auto &entry = otype_lower_bound(table, [=key](T const &x) {
        return otype_get_entry_key<K>(x) < key;
    });

    if (otype_get_entry_key<K>(entry) == key) {
        return &entry;
    } else {
        return nullptr;
    }
}


}}

