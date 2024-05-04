

#pragma once

#include "../unit/unit.hpp"

hi_export_module(hikogui.theme : tag_style);

hi_export namespace hi { inline namespace v1 {

class tag_style {
public:

    [[nodiscard]] constexpr pixels_f margin_left(pixel_density const &pd) const noexcept
    {
        return get_length(&tag_style::_margin_left, pd);
    }

    [[nodiscard]] constexpr float in_margin_left(pixel_density const &pd) const noexcept
    {
        return margin_left(pd).in(pixels);
    }

private:
    struct font_type {

        [[nodiscard]] constexpr pixels_f line_height(pixel_density const &pd) const noexcept
        {
        }
    };

    struct length_type {
        bool important;
        length_f value;
    };

    struct color_type {

    };

  
    /** Get a length.
     *
     * This is a separate function as it may need access to the font-size for this tag.
     */ 
    [[nodiscard]] constexpr pixel_f get_length(length_type tag_style::*ptr, pixel_density const &pd) const noexcept
    {
        if (auto *em_square_ptr = std::get_if<em_squares_f>(&this->*ptr)) {
            return em_square_ptr->in(em_squares) * _font.line_height(pd);

        } else if (auto *points_ptr = std::get_if<points_f>(&this->*ptr)) {
            return *points_ptr * pd;

        } else if (auto *pids_ptr = std::get_if<pids_f>(&this->*ptr)) {
            return *pids_ptr * pd;

        } else if (auto *pixel_ptr = std::get_of<pixels_f>(&this->*ptr)) {
            return *pixel_ptr;

        } else {
            hi_no_default();
        }
    }


    std::string _tag_attributes;

    std::string _name;
    std::string _id;
    std::vector<std::string> _classes;

    font_type _font;
    length_type _left_margin;
    color_type _background_color;
};

}}

