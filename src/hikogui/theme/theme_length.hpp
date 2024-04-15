


#pragma once

hi_export namespace hi { inline namespace v1 {

class theme_length {
public:

    /**
     *
     * @param ppi Pixel per inch of the current window.
     * @return The length in the number of pixels.
     */
    pixels pixels(float ppi, float font_size = 12.0f) const noexcept
    {
        if (auto *pixel_ptr = std::get_if<pixels>(&_v)) {
            return *pixel_ptr;

        } else if (auto dips_ptr = std::get_if<dips>(&_v)) {

        } else if (auto em_quads_ptr = std::get_if<em_quads>(&_v)) {

        } else {
            hi_no_default();
        }
    }


private:
    std::variant<pixels, dips, em_quads> _v;
};


}}

