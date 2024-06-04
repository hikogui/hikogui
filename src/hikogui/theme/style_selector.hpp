

#pragma once

#include "style_specificity.hpp"
#include "style_path.hpp"
#include "../macros.hpp"
#include <string>
#include <vector>

hi_export_module(hikogui.theme : style_selector);

hi_export namespace hi::inline v1 {
/**
 * @brief Represents a style selector segment used for selecting elements in a GUI theme.
 *
 */
struct style_selector_segment {
    std::string name;
    std::string id;
    std::vector<std::string> classes;
    std::vector<std::string> pseudo_classes;

    /** The next child must follow directly after this segment.
     */
    bool child_combinator = false;

    constexpr style_selector_segment() noexcept = default;
    constexpr style_selector_segment(style_selector_segment const&) = default;
    constexpr style_selector_segment(style_selector_segment&&) = default;
    constexpr style_selector_segment& operator=(style_selector_segment const&) = default;
    constexpr style_selector_segment& operator=(style_selector_segment&&) = default;

    constexpr style_selector_segment(
        std::string name,
        std::string id = {},
        std::vector<std::string> classes = {},
        std::vector<std::string> pseudo_classes = {},
        bool child_combinator = false) :
        name(std::move(name)),
        id(std::move(id)),
        classes(std::move(classes)),
        pseudo_classes(pseudo_classes),
        child_combinator(child_combinator)
    {
    }

    /**
     * Determines if a style selector segment matches a style path segment.
     *
     * @param lhs The style selector segment to compare.
     * @param rhs The style path segment to compare.
     * @return True if the style selector segment matches the style path segment, false otherwise.
     */
    [[nodiscard]] constexpr friend bool matches(style_selector_segment const& lhs, style_path_segment const& rhs) noexcept
    {
        if (not lhs.name.empty() and lhs.name != rhs.name) {
            return false;
        }

        if (not lhs.id.empty() and lhs.id != rhs.id) {
            return false;
        }

        if (lhs.classes.size() > rhs.classes.size()) {
            return false;
        }

        for (auto const& x : lhs.classes) {
            if (std::find(rhs.classes.begin(), rhs.classes.end(), x) == rhs.classes.end()) {
                return false;
            }
        }

        return true;
    }
};

/**
 * @brief Represents a style selector used for selecting elements in a GUI theme.
 *
 * The `style_selector` class is derived from `std::vector<style_selector_segment>`,
 * which allows it to store a sequence of style selector segments. It also provides
 * a method to calculate the specificity of the selector based on CSS2.2 rules.
 *
 * @note The specificity of a selector is used for calculating priorities when
 * applying styles to elements.
 */
class style_selector : public std::vector<style_selector_segment> {
    using super = std::vector<style_selector_segment>;
    using super::super;

    /**
     * @brief Calculates the specificity of the style selector.
     *
     * The specificity of a selector is calculated based on CSS2.2 rules. It takes
     * into account the presence of IDs, class names, pseudo classes, and element names
     * in the selector segments. The calculated specificity value is between 0 and 999.
     *
     * @note Based on CSS2.2: 6.4.3 Calculating a selector's specificity.
     *
     * @return The specificity value of the style selector.
     */
    [[nodiscard]] constexpr style_specificity specificity() const noexcept
    {
        auto b = size_t{};
        auto c = size_t{};
        auto d = size_t{};

        for (auto const& segment : *this) {
            b += static_cast<size_t>(not segment.id.empty());
            c += static_cast<size_t>(not segment.classes.empty());
            c += static_cast<size_t>(not segment.pseudo_classes.empty());
            d += static_cast<size_t>(not segment.name.empty());
        }

        b = std::min(b, size_t{9});
        c = std::min(c, size_t{9});
        d = std::min(d, size_t{9});
        return static_cast<style_specificity>(b * 100 + c * 10 + d);
    }
};

/**
 * @brief Checks if a style selector matches a style path.
 *
 * This function compares two ranges of iterators, `lhs_it_first` to `lhs_it_last` and `rhs_it_first` to `rhs_it_last`,
 * and determines if the style selector matches the style path. The function returns true if the style selector matches
 * the style path, and false otherwise.
 *
 * @param lhs_it_first The iterator pointing to the first element of the style selector range.
 * @param lhs_it_last The iterator pointing to the last element of the style selector range.
 * @param rhs_it_first The iterator pointing to the first element of the style path range.
 * @param rhs_it_last The iterator pointing to the last element of the style path range.
 * @return true if the style selector matches the style path, false otherwise.
 */
[[nodiscard]] constexpr bool matches(
    style_selector::const_iterator lhs_it_first,
    style_selector::const_iterator lhs_it_last,
    style_path::const_iterator rhs_it_first,
    style_path::const_iterator rhs_it_last) noexcept
{
    if (std::distance(lhs_it_first, lhs_it_last) > std::distance(rhs_it_first, rhs_it_last)) {
        return false;
    }

    while (lhs_it_first != lhs_it_last and rhs_it_first != rhs_it_last) {
        --lhs_it_last;
        --rhs_it_last;

        if (not matches(*lhs_it_last, *rhs_it_last)) {
            if (lhs_it_last->child_combinator or rhs_it_first == rhs_it_last) {
                return false;
            }
            return matches(lhs_it_first, lhs_it_last, rhs_it_first, rhs_it_last - 1);
        }
    }

    return lhs_it_last == lhs_it_first;
}

/**
 * @brief Checks if a style selector matches a style path.
 *
 * This function compares a style selector with a style path to determine if they match.
 * It uses the `matches` function to perform the comparison.
 *
 * @param lhs The style selector to compare.
 * @param rhs The style path to compare.
 * @return `true` if the style selector matches the style path, `false` otherwise.
 */
[[nodiscard]] constexpr bool matches(style_selector const& lhs, style_path const& rhs) noexcept
{
    return matches(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}
}
