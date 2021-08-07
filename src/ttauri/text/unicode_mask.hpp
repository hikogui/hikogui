

#pragma once

#include <stdint>


namespace tt {

class unicode_mask {
public:

    size_t size() const noexcept
    {
        return _size;
    }

    void add(char32_t first, char32_t last) noexcept
    {

        auto it = std::ranges::lower_bound(_entries, first, [](ttlet &item, ttlet &value) {
            return item.begin() < value;
        });

        while (first != last) {
            if (it == _entries.end()) {
                // Append the items.
                ttlet last_to_insert = std::min({last, first + entry_type::capacity});

                it = _entries.emplace(it, first, last_to_insert) + 1;
                _size += last_to_insert - first;

                first = last_to_insert;

            } else if (first < it.begin()) {
                // Insert the left side before the current entry.
                ttlet last_to_insert = std::min({last, it.begin(), first + entry_type::capacity});

                it = _entries.emplace(it, first, last_to_insert) + 1;
                _size += last_to_insert - first;

                first = last_to_insert;

            } else if (first < it.end()) {
                // Ignore the left side that overlaps with the current entry.
                first = std::min(it.end(), last);
                ++it;

            } else {
                // We are behind the current entry, skip to the next entry.
                ++it;
            }
        }
    }

    /** Optimize storage.
     */
    void optimize() noexcept
    {
        auto it = std::begin(_entries);
        if (it != std::end(_entries)) {
            auto prev_it = it++;
            while (it != std::end(_entries)) {
                if (prev_it->end() == it->begin()) {
                    ttlet move_count = std::max(it->

                }

                prev_it = it++;
            }
        }
    }

private:
    class entry_type {
    public:
        static constexpr size_t capacity = 0x7ff;

        [[nodiscard]] constexpr size_t size() const noexcept
        {
            return static_cast<size_t>(_value & 0x7ff);
        }

        [[nodiscard]] constexpr char32_t begin() const noexcept
        {
            return static_cast<char32_t>(_value >> 11);
        }

        [[nodiscard]] constexpr char32_t end() const noexcept
        {
            return begin() + static_cast<char32_t>(size());
        }

        [[nodiscard]] friend constexpr bool operator==(entry_type lhs, char32_t rhs) const noexcept
        {
            return lhs.begin() <= rhs < lhs.end();
        }

        [[nodiscard]] friend constexpr std::weak_ordering operator<=>(entry_type lhs, char32_t rhs) const noexcept
        {
            if (lhs.begin() > rhs) {
                return std::weak_ordering::greater;
            } else if (lhs.end() <= rhs) {
                return std::weak_ordering::less;
            } else {
                return std::weak_ordering::equivalent;
            }
        }

    private:
        uint32_t _value;
    };

    using entries_type = std::vector<entry_type>;
    using iterator = typename entries_type::iterator;
    using const_iterator = typename entries_type::const_iterator;

    size_t _size;
    entries_type _entries;

};

}

