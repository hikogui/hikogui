// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "box_constraints.hpp"
#include "box_shape.hpp"
#include "spreadsheet_address.hpp"
#include "../geometry/axis_aligned_rectangle.hpp"
#include "../geometry/axis.hpp"
#include "../cast.hpp"
#include <cstdint>
#include <numeric>
#include <vector>
#include <algorithm>
#include <utility>
#include <cmath>

namespace hi { inline namespace v1 {
namespace detail {

template<typename T>
struct grid_layout_cell {
    using value_type = T;

    size_t first_column = 0;
    size_t last_column = 0;
    size_t first_row = 0;
    size_t last_row = 0;
    value_type value = {};
    box_shape shape = {};

    constexpr grid_layout_cell() noexcept = default;
    constexpr grid_layout_cell(grid_layout_cell const&) noexcept = default;
    constexpr grid_layout_cell(grid_layout_cell&&) noexcept = default;
    constexpr grid_layout_cell& operator=(grid_layout_cell const&) noexcept = default;
    constexpr grid_layout_cell& operator=(grid_layout_cell&&) noexcept = default;

    constexpr grid_layout_cell(
        size_t first_column,
        size_t last_column,
        size_t first_row,
        size_t last_row,
        std::convertible_to<value_type> auto&& value) noexcept :
        first_column(first_column), last_column(last_column), first_row(first_row), last_row(last_row), value(hi_forward(value))
    {
    }

    constexpr void set_constraints(box_constraints const& constraints) noexcept
    {
        _constraints = constraints;
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr size_t first() const noexcept
    {
        if constexpr (Axis == axis::x) {
            return first_column;
        } else if constexpr (Axis == axis::y) {
            return first_row;
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr size_t last() const noexcept
    {
        if constexpr (Axis == axis::x) {
            return last_column;
        } else if constexpr (Axis == axis::y) {
            return last_row;
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr size_t span() const noexcept
    {
        return first<Axis>() - last<Axis>();
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr auto alignment() const noexcept
    {
        if constexpr (Axis == axis::x) {
            return _constraints.alignment.horizontal();
        } else if constexpr (Axis == axis::y) {
            return _constraints.alignment.vertical();
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr int minimum() const noexcept
    {
        if constexpr (Axis == axis::x) {
            return _constraints.minimum_width;
        } else if constexpr (Axis == axis::y) {
            return _constraints.minimum_height;
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr int preferred() const noexcept
    {
        if constexpr (Axis == axis::x) {
            return _constraints.preferred_width;
        } else if constexpr (Axis == axis::y) {
            return _constraints.preferred_height;
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr int maximum() const noexcept
    {
        if constexpr (Axis == axis::x) {
            return _constraints.maximum_width;
        } else if constexpr (Axis == axis::y) {
            return _constraints.maximum_height;
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr int margin_before(bool forward) const noexcept
    {
        if constexpr (Axis == axis::x) {
            if (forward) {
                return _constraints.margin_left;
            } else {
                return _constraints.margin_right;
            }
        } else if constexpr (Axis == axis::y) {
            if (forward) {
                return _constraints.margin_bottom;
            } else {
                return _constraints.margin_top;
            }
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr int margin_after(bool forward) const noexcept
    {
        if constexpr (Axis == axis::x) {
            if (forward) {
                return _constraints.margin_right;
            } else {
                return _constraints.margin_left;
            }
        } else if constexpr (Axis == axis::y) {
            if (forward) {
                return _constraints.margin_top;
            } else {
                return _constraints.margin_bottom;
            }
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr int padding_before(bool forward) const noexcept
    {
        if constexpr (Axis == axis::x) {
            if (forward) {
                return _constraints.padding_left;
            } else {
                return _constraints.padding_right;
            }
        } else if constexpr (Axis == axis::y) {
            if (forward) {
                return _constraints.padding_bottom;
            } else {
                return _constraints.padding_top;
            }
        } else {
            hi_static_no_default();
        }
    }

    template<hi::axis Axis>
    [[nodiscard]] constexpr int padding_after(bool forward) const noexcept
    {
        if constexpr (Axis == axis::x) {
            if (forward) {
                return _constraints.padding_right;
            } else {
                return _constraints.padding_left;
            }
        } else if constexpr (Axis == axis::y) {
            if (forward) {
                return _constraints.padding_top;
            } else {
                return _constraints.padding_bottom;
            }
        } else {
            hi_static_no_default();
        }
    }

private:
    box_constraints _constraints;
};

template<hi::axis Axis, typename T>
class grid_layout_axis_constraints {
public:
    constexpr static hi::axis axis = Axis;

    using value_type = T;
    using alignment_type = std::conditional_t<axis == axis::row, horizontal_alignment, vertical_alignment>;
    using cell_type = grid_layout_cell<value_type>;
    using cell_vector = std::vector<cell_type>;

    struct constraint_type {
        /** The minimum width/height of the cells perpendicular to the axis.
         */
        int minimum = 0;

        /** The preferred width/height of the cells perpendicular to the axis.
         */
        int preferred = 0;

        /** The maximum width/height of the cells perpendicular to the axis.
         */
        int maximum = std::numeric_limits<int>::max();

        /** The left/top margin of the cells perpendicular to the axis.
         */
        int margin_before = 0;

        /** The left/top padding of the cells perpendicular to the axis.
         */
        int padding_before = 0;

        /** The right/bottom padding of the cells perpendicular to the axis.
         */
        int padding_after = 0;

        /** The alignment of the cells perpendicular to the axis.
         */
        alignment_type alignment = alignment_type::none;

        /** The position of the cell.
         *
         * @note This field is valid after layout.
         */
        int pos = 0;

        /** Size of the cell.
         *
         * @note This field is valid after layout.
         */
        int size = 0;

        /** The before-position within this cell where to align to.
         *
         * @note This field is valid after layout.
         */
        std::optional<int> guideline = 0;
    };
    using constraint_vector = std::vector<constraint_type>;
    using iterator = constraint_vector::iterator;
    using const_iterator = constraint_vector::const_iterator;
    using reverse_iterator = constraint_vector::reverse_iterator;
    using reference = constraint_vector::reference;
    using const_reference = constraint_vector::const_reference;

    /** The number of cells along this axis.
     */
    size_t num = 0;

    /** The minimum width/height, excluding outer margins, of the combined cells.
     */
    int minimum = 0;

    /** The preferred width/height, excluding outer margins, of the combined cells.
     */
    int preferred = 0;

    /** The maximum width/height, excluding outer margins, of the combined cells.
     */
    int maximum = 0;

    /** The margin at the left/top.
     */
    int margin_before = 0;

    /** The margin at the right/bottom.
     */
    int margin_after = 0;

    /** The left/top padding of the combined cells.
     */
    int padding_before = 0;

    /** The right/bottom padding of the combined cells.
     */
    int padding_after = 0;

    constexpr ~grid_layout_axis_constraints() = default;
    constexpr grid_layout_axis_constraints() noexcept = default;
    constexpr grid_layout_axis_constraints(grid_layout_axis_constraints const&) noexcept = default;
    constexpr grid_layout_axis_constraints(grid_layout_axis_constraints&&) noexcept = default;
    constexpr grid_layout_axis_constraints& operator=(grid_layout_axis_constraints const&) noexcept = default;
    constexpr grid_layout_axis_constraints& operator=(grid_layout_axis_constraints&&) noexcept = default;
    [[nodiscard]] constexpr friend bool
    operator==(grid_layout_axis_constraints const&, grid_layout_axis_constraints const&) noexcept = default;

    /** Construct constraints for this axis.
     *
     * @param cell The cells
     * @param num The number of cells in the direction of the current axis
     * @param mirrored true If the axis needs to be mirrored.
     */
    constexpr grid_layout_axis_constraints(cell_vector const& cells, size_t num, bool forward) noexcept :
        _constraints(num + 1), num(num)
    {
        for (hilet& cell : cells) {
            construct_simple_cell(cell, forward);
        }
        construct_fixup();

        for (hilet& cell : cells) {
            construct_span_cell(cell);
        }
        construct_fixup();
        construct_stats();
    }

    /** Layout each cell along an axis.
     *
     * The algorithm works as follows:
     *  1. Initialize each cell based on its preferred size.
     *  2. While the grid needs to be shrunk:
     *    a. Calculate the amount of cells that are allowed to shrink.
     *    b. Apply shrinkage to the cells that are allowed to, up to the minimum.
     *    c. If all the cells are maximum shrunk, stop.
     *  3. While the grid needs to be expanded:
     *    a. Calculate the amount of cells that are allowed to expand.
     *    b. Apply expansion to the cells that are allowed to, up to the maximum.
     *    c. If all the cells are maximum expanded, goto 4.
     *  4. Expand the largest cell to make it fit.
     *
     * In an emergency widgets will get a size larger than its maximum. However
     * widgets will never get a smaller size than its minimum.
     *
     * @param size The size of the grid along its axis.
     * @param guideline_width The width of the guideline.
     * @param forward The cells are laid out in forward direction.
     */
    constexpr void layout(int size, int guideline_width, bool forward) noexcept
    {
        layout_initial();

        auto [current_size, count] = layout_shrink(begin(), end());
        while (current_size > size and count != 0) {
            // The result may shrink slightly too much, which will be fixed by expanding in the next loop.
            std::tie(current_size, count) = layout_shrink(begin(), end(), current_size - size, count);
        }

        std::tie(current_size, count) = layout_expand(begin(), end());
        while (current_size < size and count != 0) {
            // The result may expand slightly too much, we don't care.
            std::tie(current_size, count) = layout_expand(begin(), end(), size - current_size, count);
        }

        if (current_size < size and not empty()) {
            // The result may expand slightly too much, we don't care.
            back().size = back().size + size - current_size;
        }

        if (forward) {
            auto pos = 0;
            for (auto it = begin(); it != end(); ++it) {
                if (it != begin()) {
                    pos += it->margin_before;
                }
                it->pos = pos;
                it->guideline =
                    make_guideline(it->alignment, pos, pos + it->size, it->padding_before, it->padding_after, guideline_width);

                pos += it->size;
            }
        } else {
            auto pos = 0;
            for (auto it = rbegin(); it != rend(); ++it) {
                if (it != rbegin()) {
                    pos += it->margin_before;
                }
                it->pos = pos;
                it->guideline =
                    make_guideline(it->alignment, pos, pos + it->size, it->padding_before, it->padding_after, guideline_width);

                pos += it->size;
            }
        }
    }

    /** Number of cell on this axis.
     */
    [[nodiscard]] constexpr size_t size() const noexcept
    {
        hi_axiom(not _constraints.empty());
        return _constraints.size() - 1;
    }

    /** Check if this axis is empty.
     */
    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return size() != 0;
    }

    /** Iterator to the first cell on this axis.
     */
    [[nodiscard]] constexpr iterator begin() noexcept
    {
        return _constraints.begin();
    }

    /** Iterator to the first cell on this axis.
     */
    [[nodiscard]] constexpr const_iterator begin() const noexcept
    {
        return _constraints.begin();
    }

    /** Iterator to the first cell on this axis.
     */
    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return _constraints.cbegin();
    }

    /** Iterator to beyond the last cell on this axis.
     */
    [[nodiscard]] constexpr iterator end() noexcept
    {
        hi_axiom(not _constraints.empty());
        return _constraints.end() - 1;
    }

    /** Iterator to beyond the last cell on this axis.
     */
    [[nodiscard]] constexpr const_iterator end() const noexcept
    {
        hi_axiom(not _constraints.empty());
        return _constraints.end() - 1;
    }

    /** Iterator to beyond the last cell on this axis.
     */
    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        hi_axiom(not _constraints.empty());
        return _constraints.cend() - 1;
    }

    /** Iterator to the first cell on this axis.
     */
    [[nodiscard]] constexpr reverse_iterator rbegin() noexcept
    {
        return _constraints.rbegin() + 1;
    }

    /** Iterator to the first cell on this axis.
     */
    [[nodiscard]] constexpr reverse_iterator rend() noexcept
    {
        return _constraints.rend();
    }

    /** Get element.
     *
     * @note It is undefined behavior to index beyond the number of cell on this axis.
     * @param index The index of the cell.
     * @return A reference to the cell.
     */
    [[nodiscard]] constexpr reference operator[](size_t index) noexcept
    {
        hi_axiom(index < size());
        return _constraints[index];
    }

    /** Get element.
     *
     * @note It is undefined behavior to index beyond the number of cell on this axis.
     * @param index The index of the cell.
     * @return A reference to the cell.
     */
    [[nodiscard]] constexpr const_reference operator[](size_t index) const noexcept
    {
        hi_axiom(index < size());
        return _constraints[index];
    }

    /** Get the first element.
     *
     * @note It is undefined behavior to call this function on an empty axis.
     * @return A reference to the first cell.
     */
    [[nodiscard]] constexpr reference front() noexcept
    {
        hi_axiom(not empty());
        return _constraints.front();
    }

    /** Get the first element.
     *
     * @note It is undefined behavior to call this function on an empty axis.
     * @return A reference to the first cell.
     */
    [[nodiscard]] constexpr const_reference front() const noexcept
    {
        hi_axiom(not empty());
        return _constraints.front();
    }

    /** Get the last element.
     *
     * @note It is undefined behavior to call this function on an empty axis.
     * @return A reference to the last cell.
     */
    [[nodiscard]] constexpr reference back() noexcept
    {
        hi_axiom(not empty());
        return *(end() - 1);
    }

    /** Get the last element.
     *
     * @note It is undefined behavior to call this function on an empty axis.
     * @return A reference to the last cell.
     */
    [[nodiscard]] constexpr const_reference back() const noexcept
    {
        hi_axiom(not empty());
        return *(end() - 1);
    }

    /** Get the current layout position of a span.
     *
     * @note valid after layout.
     * @param first The iterator to the first cell.
     * @param last The iterator beyond the last cell.
     * @param forward The direction of the layout.
     * @return The current size of the span, including internal margins.
     */
    [[nodiscard]] constexpr int span_pos(const_iterator first, const_iterator last, bool forward) const noexcept
    {
        hi_axiom(first != last);
        if (forward) {
            return first->pos;
        } else {
            return (last - 1)->pos;
        }
    }

    /** Get the current layout position of a span.
     *
     * @note valid after layout.
     * @param first The index to the first cell.
     * @param last The index beyond the last cell.
     * @param forward The direction of the layout.
     * @return The current size of the span, including internal margins.
     */
    [[nodiscard]] constexpr int span_pos(size_t first, size_t last, bool forward) const noexcept
    {
        hi_axiom(first < last);
        hi_axiom(last <= size());
        return span_pos(cbegin() + first, cbegin() + last, forward);
    }

    /** Get the current layout size of a span.
     *
     * @note valid after layout.
     * @param first The iterator to the first cell.
     * @param last The iterator beyond the last cell.
     * @return The current size of the span, including internal margins.
     */
    [[nodiscard]] constexpr int span_size(const_iterator first, const_iterator last) const noexcept
    {
        auto r = 0;
        if (first != last) {
            r = first->size;
            for (auto it = first + 1; it != last; ++it) {
                r += it->margin_before;
                r += it->size;
            }
        }
        return r;
    }

    /** Get the current layout size of a span.
     *
     * @note valid after layout.
     * @param first The index to the first cell.
     * @param last The index beyond the last cell.
     * @return The current size of the span, including internal margins.
     */
    [[nodiscard]] constexpr int span_size(size_t first, size_t last) const noexcept
    {
        hi_axiom(first <= last);
        hi_axiom(last <= size());
        return span_size(cbegin() + first, cbegin() + last);
    }

    /** Get the minimum, preferred, maximum size of the span.
     *
     * The returned minimum, preferred and maximum include the internal margin within the span.
     *
     * @param first The iterator to the first cell.
     * @param last The iterator beyond the last cell.
     * @return The minimum, preferred and maximum size.
     */
    [[nodiscard]] constexpr std::tuple<int, int, int> const
    span_constraints(const_iterator first, const_iterator last) const noexcept
    {
        auto r_minimum = 0;
        auto r_preferred = 0;
        auto r_maximum = 0;
        auto r_margin = 0;

        if (first != last) {
            r_minimum = first->minimum;
            r_preferred = first->preferred;
            r_maximum = first->maximum;
            for (auto it = first + 1; it != last; ++it) {
                r_margin += it->margin_before;
                r_minimum += it->minimum;
                r_preferred += it->preferred;
                r_maximum += it->maximum;
            }
        }
        return {r_minimum + r_margin, r_preferred + r_margin, r_maximum + r_margin};
    }

    /** Get the minimum, preferred, maximum size of the span.
     *
     * The returned minimum, preferred and maximum include the internal margin within the span.
     *
     * @param first The index to the first cell.
     * @param last The index beyond the last cell.
     * @return The minimum, preferred and maximum size.
     */
    [[nodiscard]] constexpr std::tuple<int, int, int> const span_constraints(size_t first, size_t last) const noexcept
    {
        hi_axiom(first <= last);
        hi_axiom(last <= size());
        return span_constraints(begin() + first, begin() + last);
    }

    /** Get the minimum, preferred, maximum size of the span.
     *
     * The returned minimum, preferred and maximum include the internal margin within the span.
     *
     * @param cell The reference to the cell in the grid.
     * @return The minimum, preferred and maximum size.
     */
    [[nodiscard]] constexpr std::tuple<int, int, int> const span_constraints(cell_type const& cell) const noexcept
    {
        return span_constraints(cell.first<axis>(), cell.last<axis>());
    }

private:
    /** The constraints.
     *
     * There is one merged-constraint per cell along the axis;
     * plus one extra constraint with only `margin_before` being valid.
     *
     * Using one extra constraint reduces the amount of if-statements.
     *
     */
    constraint_vector _constraints = {};

    /** Layout initialize
     *
     * Start of with the preferred size for each cell.
     */
    constexpr void layout_initial() noexcept
    {
        for (auto& constraint : _constraints) {
            constraint.size = constraint.preferred;
        }
    }

    /** Shrink cells.
     *
     * This function is called in two different ways:
     *  - First without @a extra and @a count to get the number of pixels of the cells and the number
     *    of cells that shrink furter. These values are used to calculate @a extra and @a count
     *    of the next iteration.
     *  - Continued with @a extra and @a count filled in.
     *
     * @note Must be called after `layout_initialize()`.
     * @note It is undefined behavior to pass zero in @a count.
     * @param first The iterator to the first cell to shrink.
     * @param last The iterator to beyond the last cell to shrink.
     * @param extra The total number of pixels to shrink spread over the cells
     * @param count The number of cels between first/last that can be shrunk, from previous iteration.
     * @return Number of pixels of the cells and inner-margins, number of cells in the range that can shrink more.
     */
    [[nodiscard]] constexpr std::pair<int, size_t>
    layout_shrink(const_iterator first, const_iterator last, int extra = 0, size_t count = 1) noexcept
    {
        hilet first_ = begin() + std::distance(cbegin(), first);
        hilet last_ = begin() + std::distance(cbegin(), last);

        hi_axiom(extra >= 0);

        hilet extra_per = narrow_cast<int>((extra + count - 1) / count);

        auto new_size = 0;
        auto new_count = 0_uz;
        for (auto it = first_; it != last_; ++it) {
            it->size = it->size - std::max(extra_per, it->size - it->minimum);

            if (it != first_) {
                new_size += it->margin_before;
            }
            new_size += it->size;

            if (it->size > it->minimum) {
                ++new_count;
            }
        }

        return {new_size, new_count};
    }

    /** Expand cells.
     *
     * This function is called in two different ways:
     *  - First without @a extra and @a count to get the number of pixels of the cells and the number
     *    of cells that expand furter. These values are used to calculate @a extra and @a count
     *    of the next iteration.
     *  - Continued with @a extra and @a count filled in.
     *
     * @note Must be called after `layout_initialize()`.
     * @note It is undefined behavior to pass zero in @a count.
     * @param first The iterator to the first cell to expand.
     * @param last The iterator to beyond the last cell to expand.
     * @param extra The total number of pixels to expand spread over the cells
     * @param count The number of cels between first/last that can be expanded, from previous iteration.
     * @return Number of pixels of the cells and inner-margins, number of cells in the range that can expand more.
     */
    [[nodiscard]] constexpr std::pair<int, size_t>
    layout_expand(const_iterator first, const_iterator last, int extra = 0, size_t count = 1) noexcept
    {
        hilet first_ = begin() + std::distance(cbegin(), first);
        hilet last_ = begin() + std::distance(cbegin(), last);

        hi_axiom(extra >= 0);

        hilet extra_per = narrow_cast<int>((extra + count - 1) / count);

        auto new_size = 0;
        auto new_count = 0_uz;
        for (auto it = first_; it != last_; ++it) {
            it->size = it->size + std::min(extra_per, it->maximum - it->size);

            if (it != first_) {
                new_size += it->margin_before;
            }
            new_size += it->size;

            if (it->size < it->maximum) {
                ++new_count;
            }
        }

        return {new_size, new_count};
    }

    /** Calculate size and margins for each cell.
     *
     * @note Must be called after `layout_expand()` and `layout_shrink()`.
     */
    [[nodiscard]] constexpr int layout_size() const noexcept
    {
        auto r = 0;
        auto it = _constraints.begin();
        r += it++->size;

        hilet last = _constraints.end() - 1;
        while (it != last) {
            r += it->margin_before;
            r += it->size;
        }

        return r;
    }

    /** Construct from a simple cell.
     *
     * Calculate all the margins. And the minimum, preferred and maximum size
     * for a cell that has a span of one in the direction of the axis.
     *
     * @param cell The cell to construct.
     * @param forward Cells are ordered in forward direction (used for right-to-left languages).
     */
    constexpr void construct_simple_cell(cell_type const& cell, bool forward) noexcept
    {
        inplace_max(_constraints[cell.first<axis>()].margin_before, cell.margin_before<axis>(forward));
        inplace_max(_constraints[cell.last<axis>()].margin_before, cell.margin_after<axis>(forward));
        inplace_max(_constraints[cell.first<axis>()].padding_before, cell.padding_before<axis>(forward));
        inplace_max(_constraints[cell.last<axis>() - 1].padding_after, cell.padding_after<axis>(forward));

        if (cell.span<axis>() == 1) {
            inplace_max(_constraints[cell.first<axis>()].alignment, cell.alignment<axis>());
            inplace_max(_constraints[cell.first<axis>()].minimum, cell.minimum<axis>());
            inplace_max(_constraints[cell.first<axis>()].preferred, cell.preferred<axis>());
            inplace_min(_constraints[cell.first<axis>()].maximum, cell.maximum<axis>());
        }
    }

    /** Construct from a span-cell.
     *
     * Spread the size of a multi-span.
     *
     * @param cell The cell to construct.
     */
    constexpr void construct_span_cell(cell_type const& cell) noexcept
    {
        if (cell.span<axis>() > 1) {
            hilet[span_minimum, span_preferred, span_maximum] = span_constraints(cell);
            if (hilet extra = cell.minimum<axis>() - span_minimum; extra > 0) {
                hilet extra_per_cell = narrow_cast<int>((extra + cell.span<axis>() - 1) / cell.span<axis>());
                for (auto i = cell.first<axis>(); i != cell.last<axis>(); ++i) {
                    _constraints[i].minimum += extra_per_cell;
                }
            }

            if (hilet extra = cell.preferred<axis>() - span_preferred; extra > 0) {
                hilet extra_per_cell = narrow_cast<int>((extra + cell.span<axis>() - 1) / cell.span<axis>());
                for (auto i = cell.first<axis>(); i != cell.last<axis>(); ++i) {
                    _constraints[i].preferred += extra_per_cell;
                }
            }

            if (hilet extra = cell.maximum<axis>() - span_preferred; extra < 0) {
                hilet extra_per_cell = narrow_cast<int>((extra + cell.span<axis>() - 1) / cell.span<axis>());
                for (auto i = cell.first<axis>(); i != cell.last<axis>(); ++i) {
                    // The maximum could become too low here, fixup() will fix this.
                    _constraints[i].maximum += extra_per_cell;
                }
            }
        }
    }

    /** Construct fixup.
     *
     * Fixup minimum, preferred, maimum. And calculate the padding.
     */
    constexpr void construct_fixup() noexcept
    {
        for (auto& row : _constraints) {
            inplace_max(row.preferred, row.minimum);
            inplace_max(row.maximum, row.preferred);
            if (row.padding_before + row.padding_after > row.minimum) {
                hilet padding_diff = row.padding_after - row.padding_before;
                hilet middle = std::clamp(row.minimum / 2 + padding_diff, 0, row.minimum);
                row.padding_after = middle;
                row.padding_before = row.minimum - middle;
            }
        }
    }

    /** Calculate the margin and padding before and after.
     */
    constexpr void construct_stats() noexcept
    {
        std::tie(minimum, preferred, maximum) = span_constraints(0, num);
        margin_before = _constraints.front().margin_before;
        margin_after = _constraints.back().margin_before;
        padding_before = _constraints.front().padding_before;
        padding_after = _constraints[_constraints.size() - 2].padding_after;
    }
};

} // namespace detail

/** Grid layout algorithm.
 *
 *
 *
 */
template<typename T>
class grid_layout {
public:
    using value_type = T;

    using cell_type = detail::grid_layout_cell<value_type>;
    using cell_vector = std::vector<cell_type>;
    using iterator = cell_vector::iterator;
    using const_iterator = cell_vector::const_iterator;
    using reference = cell_vector::reference;

    ~grid_layout() = default;
    constexpr grid_layout() noexcept = default;
    constexpr grid_layout(grid_layout const&) noexcept = default;
    constexpr grid_layout(grid_layout&&) noexcept = default;
    constexpr grid_layout& operator=(grid_layout const&) noexcept = default;
    constexpr grid_layout& operator=(grid_layout&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(grid_layout const&, grid_layout const&) noexcept = default;

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _cells.empty();
    }

    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return _cells.size();
    }

    [[nodiscard]] constexpr size_t num_columns() const noexcept
    {
        return _num_columns;
    }

    [[nodiscard]] constexpr size_t num_rows() const noexcept
    {
        return _num_rows;
    }

    [[nodiscard]] constexpr iterator begin() noexcept
    {
        return _cells.begin();
    }

    [[nodiscard]] constexpr iterator end() noexcept
    {
        return _cells.begin();
    }

    [[nodiscard]] constexpr const_iterator begin() const noexcept
    {
        return _cells.begin();
    }

    [[nodiscard]] constexpr const_iterator end() const noexcept
    {
        return _cells.begin();
    }

    [[nodiscard]] constexpr const_iterator cbegin() const noexcept
    {
        return _cells.cbegin();
    }

    [[nodiscard]] constexpr const_iterator cend() const noexcept
    {
        return _cells.cbegin();
    }

    /** Check if the cell on the grid is already in use.
     *
     * @param first_column The first column of the cell-span.
     * @param last_column One beyond the last column of the cell-span.
     * @param first_row The first row of the cell-span.
     * @param last_row One beyond the last row of the cell-span.
     * @retval true If the given cell-span overlaps with an already existing cell.
     */
    [[nodiscard]] constexpr bool cell_in_use(size_t first_column, size_t last_column, size_t first_row, size_t last_row) noexcept
    {
        // At least one cell must be in the range.
        hi_axiom(first_column < last_column);
        hi_axiom(first_row < last_row);

        for (hilet& cell : _cells) {
            if (first_column >= cell.last_column) {
                continue;
            }
            if (last_column <= cell.first_column) {
                continue;
            }
            if (first_row >= cell.last_row) {
                continue;
            }
            if (last_row <= cell.first_row) {
                continue;
            }
            return true;
        }
        return false;
    }

    /** Check if the cell on the grid is already in use.
     *
     * @param first_column The first column of the cell-span.
     * @param last_column One beyond the last column of the cell-span.
     * @param first_row The first row of the cell-span.
     * @param last_row One beyond the last row of the cell-span.
     * @param value The value to be copied or moved into the cell.
     * @return A reference to the created cell.
     */
    template<forward_of<value_type> Value>
    constexpr reference
    add_cell(size_t first_column, size_t last_column, size_t first_row, size_t last_row, Value&& value) noexcept
    {
        // At least one cell must be in the range.
        hi_axiom(first_column < last_column);
        hi_axiom(first_row < last_row);
        hi_axiom(not cell_in_use(first_column, last_column, first_row, last_row));
        auto& r = _cells.emplace_back(first_column, last_column, first_row, last_row, std::forward<Value>(value));
        update_after_insert_or_delete();
        return r;
    }

    /** Check if the cell on the grid is already in use.
     *
     * @param column The column of the cell.
     * @param row The row of the cell.
     * @param value The value to be copied or moved into the cell.
     * @return A reference to the created cell.
     */
    template<forward_of<value_type> Value>
    constexpr reference add_cell(size_t column, size_t row, Value&& value) noexcept
    {
        return add_cell(column, column + 1, row, row + 1, std::forward<Value>(value));
    }

    constexpr void clear() noexcept
    {
        _cells.clear();
        update_after_insert_or_delete();
    }

    [[nodiscard]] constexpr box_constraints get_constraints(bool left_to_right) const noexcept
    {
        // Rows in the grid are laid out from top to bottom which is reverse from the y-axis up.
        _row_constraints = {_cells, num_rows(), false};
        _column_constraints = {_cells, num_columns(), left_to_right};

        auto r = box_constraints{};
        r.minimum_width = _column_constraints.minimum;
        r.preferred_width = _column_constraints.preferred;
        r.maximum_width = _column_constraints.maximum;
        r.margin_left = left_to_right ? _column_constraints.margin_before : _column_constraints.margin_after;
        r.margin_right = left_to_right ? _column_constraints.margin_after : _column_constraints.margin_before;
        r.padding_left = left_to_right ? _column_constraints.padding_before : _column_constraints.padding_after;
        r.padding_right = left_to_right ? _column_constraints.padding_after : _column_constraints.padding_before;

        r.minimum_height = _row_constraints.minimum;
        r.preferred_height = _row_constraints.preferred;
        r.maximum_height = _row_constraints.maximum;
        r.margin_bottom = _row_constraints.margin_after;
        r.margin_top = _row_constraints.margin_before;
        r.padding_bottom = _row_constraints.padding_after;
        r.padding_top = _row_constraints.padding_before;

        r.alignment = [&] {
            if (num_rows() == 1 and num_columns() == 1) {
                return hi::alignment{_column_constraints.front().alignment, _row_constraints.front().alignment};
            } else if (num_rows() == 1) {
                return hi::alignment{_row_constraints.front().alignment};
            } else if (num_columns() == 1) {
                return hi::alignment{_column_constraints.front().alignment};
            } else {
                return hi::alignment{};
            }
        }();

        return r;
    }

    constexpr void set_layout(int width, int height, int guideline_width, bool left_to_right) noexcept
    {
        // Rows in the grid are laid out from top to bottom which is reverse from the y-axis up.
        _row_constraints.layout(width, guideline_width, false);
        _column_constraints.layout(height, 0, left_to_right);

        // Assign the shape for each cell.
        for (auto& cell : _cells) {
            cell.shape.left = _column_constraints.span_pos(cell.first_column, cell.last_column, left_to_right);
            cell.shape.bottom = _row_constraints.span_pos(cell.first_row, cell.last_row, false);
            cell.shape.right = cell.shape.left + _column_constraints.span_size(cell.first_column, cell.last_column);
            cell.shape.top = cell.shape.bottom + _row_constraints.span_size(cell.first_row, cell.last_row);
            cell.shape.baseline = cell.span<axis::row>() == 1 ? _row_constraints[cell.first_row].guideline : std::nullopt;
            cell.shape.centerline =
                cell.span<axis::column>() == 1 ? _column_constraints[cell.first_column].guideline : std::nullopt;
        }
    }

private:
    cell_vector _cells = {};
    size_t _num_rows = 0;
    size_t _num_columns = 0;
    mutable detail::grid_layout_axis_constraints<axis::row, value_type> _row_constraints = {};
    mutable detail::grid_layout_axis_constraints<axis::column, value_type> _column_constraints = {};

    /** Sort the cells ordered by row then column.
     *
     * The ordering is the same as they keyboard focus chain order.
     */
    constexpr void sort_cells() noexcept
    {
        std::sort(_cells.begin(), _cells.end(), [](cell_type const& lhs, cell_type const& rhs) {
            if (lhs.first_row != rhs.first_row) {
                return lhs.first_row < rhs.first_row;
            } else {
                return lhs.first_column < rhs.first_column;
            }
        });
    }

    /** Updates needed after a cell was added or removed.
     */
    constexpr void update_after_insert_or_delete() noexcept
    {
        sort_cells();

        _num_rows = 0;
        _num_columns = 0;
        for (hilet& cell : _cells) {
            inplace_max(_num_rows, cell.last_row);
            inplace_max(_num_columns, cell.last_column);
        }
    }
};
}} // namespace hi::v1
