

#pragma once

#include "../geometry/module.hpp"

namespace hi { inline namespace v1 {

template<hi::axis Axis>
class grid_axis {
public:

    constexpr void clear() noexcept
    {
        _entries.clear();
    }

    constexpr void resize(size_t n) noexcept
    {
        _entries.resize(n);
    }

    constexpr void set(size_t index, float minimum, float maximum. float margin_before, float margin_after) noexcept
    {
        hi_axiom(not _entries.empty());
        hi_axiom_bounds(index, _entries);

        auto &entry = _entries[index];
        inplace_max(entry.minimum, minimum);
        inplace_min(entry.maximum, maximum);
        inplace_max(entry.margin_before, margin_before);
        if (index + 1 < _entries.size()) {
            inplace_max(_entries[index + 1], margin_after);
        }
    }

private:
    struct entry_type {
        float minimum;
        float maximum;
        float margin_before;
        float margin_after;
    };

    std::vector<entry_type> _entries;
};


}}

