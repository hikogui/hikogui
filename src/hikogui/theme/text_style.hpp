

#pragma once

#include "../unicode/unicode.hpp"
#include "../font/font.hpp"
#include "../i18n/i18n.hpp"
#include "../units/units.hpp"

hi_export_module(hikogui.theme : text_style);

hi_export namespace hi { inline namespace v1 {

/** A cascading text style.
 * 
*/
class text_style {
public:
    struct style_type {
        hi::font *font;
        hi::font_size_s size;  
        hi::color color;  
    };

    constexpr text_style() noexcept = default;
    constexpr text_style(text_style const &) noexcept = default;
    constexpr text_style(text_style &&) noexcept = default;
    constexpr text_style &operator=(text_style const &) noexcept = default;
    constexpr text_style &operator=(text_style &&) noexcept = default;

    constexpr void add_instruction(hi::phrasing_mask phrasing, iso_639 language, iso_15924 script, iso_3166 region, )

    /** Compute the specific text-style for the language and phrasing.
     */
    [[nodiscard]] constexpr style_type operator()(hi::phrasing phrasing, iso_639 language, iso_15924 script, iso_3166 region) const noexcept
    {
        auto r = style_type{};

        bool font_important = false;
        bool size_important = false;
        bool color_important = false;
        for (auto const &instruction : _instructions) {
            if (instruction.matches(phrasing, language, script, region)) {
                if (instruction.font and (not font_important or instruction.font_important)) {
                    font_important = font_important or instruction.font_important;
                    r.font = instruction.font;
                }

                if (instruction.size and (not size_important or instruction.size_important)) {
                    size_important = size_important or instruction.size_important;
                    r.size = instruction.size;
                }

                if (instruction.color and (not color_important or instruction.color_important)) {
                    color_important = color_important or instruction.color_important;
                    r.color = instruction.color;
                }
            }
        }

        hi_assert_not_null(r.font);
        return r;
    }

private:
    struct text_style_instruction {
        hi::phrasing_mask phrasing_mask;
        iso_639 language_mask;
        iso_15924 script_mask;
        iso_3166 region_mask;

        hi::font *font;
        std::optional<hi::font_size_s> size;  
        std::optional<hi::color> color;

        bool font_important;
        bool size_important;
        bool color_important;

        /** Check if this instruction matches the phrasing and language.
         */
        [[nodiscard]] constexpr bool matches(hi::phrasing phrasing, iso_639 language, iso_15924 script, iso_3166 region) const noexcept
        {
            return matches(phrasing_mask, phrasing) and matches(language_mask, language) and matches(script_mask, script) and matches(region_mask, region);
        }
    };

    std::vector<text_style_instruction> _instructions;
};

}}
