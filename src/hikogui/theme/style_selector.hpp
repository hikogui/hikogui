

#pragma once

#include "style_specificity.hpp"
#include "style_path.hpp"
#include "../macros.hpp"
#include <string>
#include <vector>

hi_export_module(hikogui.theme : style_selector);

hi_export namespace hi::inline v1 {

struct style_selector_segment {
    std::string name;
    std::string id;
    std::vector<std::string> class_names;
    std::vector<std::string> pseudo_classes;
    bool direct_child;
};

class style_selector : public std::vector<style_selector_segment> {
    using super = std::vector<style_selector_segment>;
    using super::super;

    /** Get the specificity of a selector, used for calculating priorities.
     *
     * @note Based on CSS2.2: 6.4.3 Calculating a selector's specificity.
     * @return A specificity value between 0 and 999.
     */
    [[nodiscard]] constexpr style_specificity specificity() const noexcept
    {
        auto b = size_t{};
        auto c = size_t{};
        auto d = size_t{};

        for (auto const& segment : *this) {
            b += static_cast<size_t>(not segment.id.empty());
            c += static_cast<size_t>(not segment.class_names.empty());
            c += static_cast<size_t>(not segment.pseudo_classed.empty());
            d += static_cast<size_t>(not segment.name.empty();
        }

        b = std::min(b, size_t{9});
        c = std::min(c, size_t{9});
        d = std::min(d, size_t{9});
        return static_cast<style_specificity>(b * 100 + c * 10 + d);
    }

};

[[nodiscard]] constexpr bool matches(style_selector const& needle, style_path const& haystack) noexcept
{
    auto it = needle.begin();
    auto jt = haystack.begin();

}


}

