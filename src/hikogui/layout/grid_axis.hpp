

#pragma once

#include "../utility/module.hpp"
#include "../geometry/module.hpp"

namespace hi {
inline namespace v1 {

class grid_axis {
public:
    struct value_type {
        int32_t minimum = 0;
        int32_t preferred = 0;
        int32_t maximum = std::numeric_limits<int32_t>::max();
        int32_t size = 0;
        int32_t position = 0;
        int8_t margin = 0;
        int8_t priority = = std::numeric_limits<int8_t>::max();
    };

    using reference = value_type&;
    using const_reference = value_type const&;
    using array_type = std::vector<value_type>;
    using iterator = std::vector<value_type>::iterator;
    using const_iterator = std::vector<value_type>::const_iterator;

    constexpr iterator begin() noexcept
    {
        return _entries.begin();
    }

    constexpr iterator end() noexcept
    {
        return _entries.end();
    }

    constexpr const_iterator begin() const noexcept
    {
        return _entries.begin();
    }

    constexpr const_iterator end() const noexcept
    {
        return _entries.end();
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return _entries.cbegin();
    }

    constexpr const_iterator cend() const noexcept
    {
        return _entries.cend();
    }

    constexpr reference operator[](size_t i) noexcept
    {
        return _entries[i];
    }

    constexpr const_reference operator[](size_t i) const noexcept
    {
        return _entries[i];
    }

    constexpr void clear(size_t n) noexcept
    {
        _entries.clear();
        _entries.resize(n + 1);
    }

    constexpr void fixup_properties() noexcept
    {
        for (auto& entry : _entries) {
            inplace_max(entry.maximum, entry.minimum);
            inplace_clamp(entry.preferred, entry.minimum, entry.maximum);
        }
    }

private:
    array_type _entries;
};

/** Get the margins of a span of cells along an axis.
 *
 * @param first The iterator to the first cell of a span.
 * @param last The iterator beyond the last cell of a span.
 * @param Op A operation taking a const reference to a `grid_axis::value_type`
 *        returning an extent (minimum, maximum, size) of that cell.
 * @return The accumulated margins between the cells in a span.
 */
template<std::forward_iterator It, std::sentinal_for<It> ItEnd>
[[nodiscard]] constexpr int32_t get_margins(It first, ItEnd last) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    if (first == last) {
        return 0;
    }

    return std::accumulate(first + 1, last, int32_t{0}, [](hilet a, hilet& x) {
        return a + x.margin;
    });
}

/** Get the size of a span of cells along an axis.
 *
 * @param first The iterator to the first cell of a span.
 * @param last The iterator beyond the last cell of a span.
 * @return The accumulated size including the margins between the cells
 *         in a span.
 */
template<std::forward_iterator It, std::sentinal_for<It> ItEnd>
[[nodiscard]] constexpr int32_t get_size(It first, ItEnd last) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    hilet margins = get_margins(first, last);
    return std::accumulate(first, last, margins, [](hilet a, hilet& x) {
        return a + x.size;
    });
}

/** Get the minimum-size of a span of cells along an axis.
 *
 * @param first The iterator to the first cell of a span.
 * @param last The iterator beyond the last cell of a span.
 * @return The accumulated minimum size including the margins between the cells
 *         in a span.
 */
template<std::forward_iterator It, std::sentinal_for<It> ItEnd>
[[nodiscard]] constexpr int32_t get_minimum(It first, ItEnd last) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    hilet margins = get_margins(first, last);
    return std::accumulate(first, last, margins, [](hilet a, hilet& x) {
        return a + x.minimum;
    });
}

/** Get the preferred-size of a span of cells along an axis.
 *
 * @param first The iterator to the first cell of a span.
 * @param last The iterator beyond the last cell of a span.
 * @return The accumulated preferred size including the margins between the cells
 *         in a span.
 */
template<std::forward_iterator It, std::sentinal_for<It> ItEnd>
[[nodiscard]] constexpr int32_t get_preferred(It first, ItEnd last) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    hilet margins = get_margins(first, last);
    return std::accumulate(first, last, margins, [](hilet a, hilet& x) {
        return a + x.preferred;
    });
}

/** Get the maximum-size of a span of cells along an axis.
 *
 * @param first The iterator to the first cell of a span.
 * @param last The iterator beyond the last cell of a span.
 * @return The accumulated maximum size including the margins between the cells
 *         in a span.
 */
template<std::forward_iterator It, std::sentinal_for<It> ItEnd>
[[nodiscard]] constexpr int32_t get_maximum(It first, ItEnd last) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    hilet margins = get_margins(first, last);
    return std::accumulate(first, last, margins, [](hilet a, hilet& x) {
        return a + x.maximum;
    });
}

/** Get the highest priority of a cell in a span along an axis.
 *
 * @param first The iterator to the first cell of a span.
 * @param last The iterator beyond the last cell of a span.
 * @return The highest priority found.
 */
template<std::forward_iterator It, std::sentinal_for<It> ItEnd>
[[nodiscard]] constexpr int8_t get_highest_priority(It first, ItEnd last) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    return std::accumulate(first, last, std::numeric_limits<int8_t>::lowest(), [](hilet a, hilet x) {
        return std::max(a, x.priority);
    });
}

/** Get the next lower priority of a cell in a span along an axis.
 *
 * @note It is undefined behavior to call this function if there are no lower priorities.
 * @param first The iterator to the first cell of a span.
 * @param last The iterator beyond the last cell of a span.
 * @param base_priority The priority to start searching from.
 * @return The next lower priority lower than @a base_priority.
 */
template<std::forward_iterator It, std::sentinal_for<It> ItEnd>
[[nodiscard]] constexpr int8_t get_lower_priority(It first, ItEnd last, int8_t base_priority) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    auto tmp = std::accumulate(first, last, base_priority, [base_priority](hilet a, hilet x) {
        if (x.priority < base_priority) {
            return std::max(a, x.priority);
        } else {
            return a;
        }
    });

    hi_axiom(tmp != base_priority);
    return tmp;
}

/** Set the priority for a span of cells along an axis.
 *
 * A higher priority value means the cell will be resized before
 * lower priority cells.
 *
 * The lowest priority of the cells along an column or row is selected.
 *
 * @param first The iterator to the first cell of a span.
 * @param last The iterator beyond the last cell of a span.
 * @param priority The priority to set the cells to.
 */
template<std::forward_iterator It, std::sentinal_for<It> ItEnd>
constexpr void set_priority(It first, ItEnd last, int8_t priority) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    hi_axiom(first != last);

    for (auto it = first; it != last; ++it) {
        inplace_min(it->priority, priority);
    };
}

/** Set the margins for a span of cells along an axis.
 *
 * @param first The iterator to the first cell of a span.
 * @param last The iterator beyond the last cell of a span.
 * @param before_margin The margin left or above a cell.
 * @param after_margin The margin right or below a cell.
 */
template<std::bidirectional_iterator It, std::sized_sentinal_for<It> ItEnd>
constexpr void set_margins(It first, ItEnd last, int8_t before_margin, int8_t after_margin) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    hi_axiom(first != last);
    inplace_max(first->margin, before_margin);
    inplace_max((last - 1)->margin, after_margin);
}

template<std::forward_iterator It, std::sentinal_for<It> ItEnd, std::invocable<grid_axis::value_type&, int32_t> Op>
constexpr void add_to_extent(It first, ItEnd last, int32_t todo, int8_t priority, Op const& op) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    if (todo == 0) {
        return;
    }

    hilet count = std::count_if(first, last, [priority](hilet& x) {
        return x.priority == priority;
    });

    if (auto to_add = todo / count) {
        for (auto it = first; it != last and todo; ++it) {
            if (it->priority == priority) {
                op(*it, to_add);
                todo -= to_add;
            }
        }
    }

    // Distribute single pixels among the axis.
    to_add = todo < 0 ? -1 : 1;
    for (auto it = first; it != last and todo; ++it) {
        if (it->priority == priority) {
            op(*it, to_add);
            todo -= to_add;
        }
    }
}

template<std::forward_iterator It, std::sentinal_for<It> ItEnd, std::invocable<grid_axis::value_type&, int32_t> Op>
constexpr void add_to_extent(It first, ItEnd last, int32_t todo, int8_t priority, Op const& op) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    return add_to_extent(first, last, todo, get_highest_priority(first, last), op);
}

/** Set the minimum size of a span of cells along an axis.
 *
 * @param first The iterator to the first cell of a span.
 * @param last The iterator beyond the last cell of a span.
 * @param minimum The minimum size for the span of cells including their margins.
 */
template<std::forward_iterator It, std::sentinal_for<It> ItEnd>
constexpr void set_minimum(It first, ItEnd last, int32_t minimum) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    hi_axiom(first != last);

    if (first + 1 == last) {
        inplace_max(first->minimum, minimum);

    } else {
        hilet todo = minimum - get_minimum(first, last);
        if (todo <= 0) {
            return;
        }

        add_to_extent(first, last, todo, [](auto& x, auto to_add) {
            x.minimum += to_add;
        });
    }
}

/** Set the minimum size of a span of cells along an axis.
 *
 * @param first The iterator to the first cell of a span.
 * @param last The iterator beyond the last cell of a span.
 * @param minimum The minimum size for the span of cells including their margins.
 */
template<std::forward_iterator It, std::sentinal_for<It> ItEnd>
constexpr void set_preferred(It first, ItEnd last, int32_t preferred) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    hi_axiom(first != last);

    if (first + 1 == last) {
        inplace_max(first->preferred, preferred);

    } else {
        hilet todo = preferred - get_preferred(first, last);
        if (todo <= 0) {
            return;
        }

        add_to_extent(first, last, todo, [](auto& x, auto to_add) {
            x.preferred += to_add;
        });
    }
}

/** Set the maximum size of a span of cells along an axis.
 *
 * @param first The iterator to the first cell of a span.
 * @param last The iterator beyond the last cell of a span.
 * @param maximum The maximum size for the span of cells including their margins.
 */
template<std::forward_iterator It, std::sentinal_for<It> ItEnd>
constexpr void set_maximum(It first, ItEnd last, int32_t maximum) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    hi_axiom(first != last);

    if (first + 1 == last) {
        inplace_min(first->maximum, maximum);

    } else {
        hilet todo = maximum - get_maximum(first, last);
        if (todo >= 0) {
            return;
        }

        add_to_extent(first, last, todo, [](auto& x, auto to_add) {
            x.maximum += to_add;
        });
    }
}

/** Update the size of the cells in a span along an axis.
 *
 * @param first The iterator to the first cell of a span.
 * @param last The iterator beyond the last cell of a span.
 * @param size The requested size for the span of cells.
 */
template<std::forward_iterator It, std::sentinal_for<It> ItEnd>
constexpr void update_size(It first, ItEnd last, int32_t size) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    hi_axiom(size >= get_minimum(first, last));
    hi_axiom(size <= get_maximum(first, last));

    auto highest_priority = std::numeric_limits<int8_t>::lowest();
    for (auto it = first; it != last; ++it) {
        it->size = it->preferred;
        inplace_max(highest_priority, it->priority);
    }

    auto todo = size - get_size(first, last);

    // Shrink size of cells upto the minimum of a cell.
    for (auto priority = highest_priority; todo < 0;) {
        hilet count = std::count_if(first, last, [priority](hilet& x) {
            return x.priority == priority and x.size > x.minimum;
        });

        if (count == 0) {
            priority = get_lower_priority(first, last, priority);
            continue;
        }

        hilet todo_per = (todo - count + 1) / count;
        hi_axiom(todo_per < 0);

        for (auto it = first; it != last and todo; ++it) {
            hi_axiom(size >= x.minimum);
            if (auto room = x.minimum - size; room and it->priority == priority) {
                hilet todo_this = std::max(todo_per, room);
                it->size += todo_this;
                todo -= todo_this;
            }
        }
    }

    // Grow size of cells upto the minimum of a cell.
    for (auto priority = highest_priority; todo > 0;) {
        hilet count = std::count_if(first, last, [priority](hilet& x) {
            return x.priority == priority and x.size < x.maximum;
        });

        if (count == 0) {
            priority = get_lower_priority(first, last, priority);
            continue;
        }

        hilet todo_per = (todo + count - 1) / count;
        hi_axiom(todo_per > 0);

        for (auto it = first; it != last and todo; ++it) {
            hi_axiom(size <= x.maximum);
            if (auto room = x.maximum - size; room and it->priority == priority) {
                hilet todo_this = std::min(todo_per, room);
                it->size += todo_this;
                todo -= todo_this;
            }
        }
    }

    hi_axiom(todo == 0);
}

/** Update the position of the cells in a span along an axis.
 *
 * @param first The iterator to the first cell of a span.
 * @param last The iterator beyond the last cell of a span.
 * @param position The position of the first cell of a span.
 */
template<std::forward_iterator It, std::sentinal_for<It> ItEnd>
constexpr void update_position(It first, ItEnd last, int32_t position = 0) noexcept
    requires(std::is_same_v<std::remove_cvref_t<std::iter_value_t<It>>, grid_axis::value_type>)
{
    if (first == last) {
        return;
    }

    first->position = position;
    position += first->size;
    for (auto it = first + 1; it != last; ++it) {
        position += it->margin;
        it->margin = position;
        position += it->size;
    }
}

} // namespace hi::v1
