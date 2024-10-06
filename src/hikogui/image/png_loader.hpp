
#pragma once

#include "../macros.hpp"
#include "sfloat_rgba16.hpp"
#include "pixmap.hpp"
#include <gsl/gsl>
#include <png.h>
#include <span>
#include <string>
#include <string_view>
#include <cstddef>
#include <cstdint>
#include <csetjmp>
#include <filesystem>
#include <functional>
#include <system_error>

hi_warning_push();
// interaction between '_setjmp' and C++ object destruction is non-portable.
// This warning is caused by using setjmp/longjmp in C++ code, even when it is
// used correctly.
hi_warning_ignore_msvc(4611);

hi_export_module(hikogui.image : png_loader);

hi_export namespace hi::inline v1 {

enum class png_interlace_method {
    none,
    adam7
};

enum class png_color_type {
    gray,
    gray_alpha,
    palette,
    rgb,
    rgb_alpha
};

class png_loader;

class png_loader_delegate {
public:
    virtual std::error_code info(png_loader *sender) noexcept = 0;

    virtual std::error_code row(png_loader *sender, std::span<sfloat_rgba16> row, size_t row_nr, unsigned int pass) noexcept = 0;

    virtual std::error_code end(png_loader *sender) noexcept = 0;

    virtual void user_error(png_loader *sender, std::string_view msg) noexcept = 0;
    virtual void user_warning(png_loader *sender, std::string_view msg) noexcept = 0;
};


class png_loader {
public:
    ~png_loader()
    {
        _cleanup();
    }

    png_loader(png_loader_delegate *delegate) : _delegate(delegate)
    {
        assert(_delegate != nullptr);

        _png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, this, _user_error_fn, _user_warning_fn);
        if (_png_ptr == nullptr) {
            _cleanup();
            throw std::runtime_error("png_create_read_struct");
        }
    }

    /** Set the maximum size of the image.
     * 
     * This is useful to prevent loading of images that are too large.
     * 
     * @param width The maximum width of the image.
     * @param height The maximum height of the image.
     */
    void set_limit(size_t width, size_t height)
    {
        if (std::setjmp(png_jmpbuf(_png_ptr))) {
            throw std::runtime_error(_last_error_message);
        }

        png_set_user_limits(_png_ptr, gsl::narrow<png_uint_32>(width), gsl::narrow<png_uint_32>(height));
    }

    /** Specify how many bytes you have already read from the file.
     *
     * This is useful if you read png-data from a file using a method that
     * does not include an ability to seek.
     *
     * @param nr_bytes The number of bytes at the start of the png-data
     *                 which will not be included in calls to `add_data()`.
     */
    void skip_signature(size_t nr_bytes)
    {
        if (std::setjmp(png_jmpbuf(_png_ptr))) {
            throw std::runtime_error(_last_error_message);
        }

        png_set_sig_bytes(_png_ptr, gsl::narrow<int>(nr_bytes));
    }

    void add_data(std::span<std::byte const> data)
    {
        if (std::setjmp(png_jmpbuf(_png_ptr))) {
            throw std::runtime_error(_last_error_message);
        }

        if (_info_ptr == nullptr) {
            // Delayed initialization of info_ptr and progressive read.
            // So that we don't need to call std::setjmp() in the constructor.
            png_set_progressive_read_fn(_png_ptr, this, _info_callback, _row_callback, _end_callback);

            _info_ptr = png_create_info_struct(_png_ptr);
            if (_info_ptr == nullptr) {
                png_error(_png_ptr, "PNG: Could not create info struct.");
            }
        }

        auto const bytes = reinterpret_cast<png_bytep>(const_cast<std::byte *>(data.data()));
        png_process_data(_png_ptr, _info_ptr, bytes, data.size());
    }

    /** Interlace method of image.
     *
     * Interlace methods:
     *  - none: 1 pass, all data in one go.
     *  - adam7: 7 passes.
     *    + 1: x=%8+0, y=%8+0
     *    + 2: x=%8+4, y=%8+0
     *    + 3: x=%4+0, y=%8+4
     *    + 4: x=%4+2, y=%4+0
     *    + 5: x=%2+0, y=%4+2
     *    + 6: x=%2+1, y=%2+0
     *    + 7: x=%1+0, y=%2+1
     *
     * ```
     *   01234567
     *  +--------
     * 0|1646264
     * 1|7777777
     * 2|5656565
     * 3|7777777
     * 4|3646364
     * 5|7777777
     * 6|5656565
     * 7|7777777
     * ```
     */
    [[nodiscard]] png_interlace_method interlace_method() const noexcept
    {
        return _interlace_method;
    }

private:
    png_loader_delegate *_delegate = nullptr;
    png_structp _png_ptr = nullptr;
    png_infop _info_ptr = nullptr;
    std::string _last_error_message = {};
    size_t _width = 0;
    size_t _height = 0;
    int _bit_depth = 0;
    png_color_type _color_type = png_color_type::gray;
    png_interlace_method _interlace_method = png_interlace_method::none;
    int _compression_method = 0;
    int _filter_method = 0;
    std::vector<color> _row;

    static void _info_callback(png_structp png_ptr, png_infop info) noexcept
    {
        // setjmp has already been setup, as this a callback.

        auto const _self = png_get_progressive_ptr(png_ptr);
        assert(_self != nullptr);
        auto const self = std::launder(static_cast<png_loader *>(_self));

        self->_width = png_get_image_width(png_ptr, info);
        self->_height = png_get_image_height(png_ptr, info);
        self->_bit_depth = png_get_bit_depth(png_ptr, info);

        switch (png_get_interlace_type(png_ptr, info)) {
        case PNG_INTERLACE_NONE:
            self->_interlace_method = png_interlace_method::none;
            break;
        case PNG_INTERLACE_ADAM7:
            self->_interlace_method = png_interlace_method::adam7;
            break;
        default:
            png_error(png_ptr, "PNG: unknown interlace method");
        }

        switch (png_get_color_type(png_ptr, info)) {
        case PNG_COLOR_TYPE_GRAY:
            self->_color_type = png_color_type::gray;
            break;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            self->_color_type = png_color_type::gray_alpha;
            break;
        case PNG_COLOR_TYPE_PALETTE:
            self->_color_type = png_color_type::palette;
            break;
        case PNG_COLOR_TYPE_RGB:
            self->_color_type = png_color_type::rgb;
            break;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            self->_color_type = png_color_type::rgb_alpha;
            break;
        default:
            png_error(png_ptr, "PNG: unknown color type");
        }

        // First convert palette and gray images to RGB (with optional alpha)
        if (self->_color_type == png_color_type::palette) {
            png_set_palette_to_rgb(png_ptr);

        } else if (self->_color_type == png_color_type::gray or self->_color_type == png_color_type::gray_alpha) {
            if (self->_bit_depth < 8) {
                png_set_expand_gray_1_2_4_to_8(png_ptr);
            }
            png_set_gray_to_rgb(png_ptr);
        }

        // Expand the image to 16-bit per channel.
        if (self->_bit_depth < 16) {
            // Expand the image to 16-bit per channel.
            // This is needed to have enough precision for conversion to linear space.
            png_set_expand_16(png_ptr);
        }

        // If the image is missing an alpha channel, then add one.
        if (png_get_valid(png_ptr, info, PNG_INFO_tRNS)) {
            png_set_tRNS_to_alpha(png_ptr);
        } else if (self->_color_type == png_color_type::rgb) {
            png_set_add_alpha(png_ptr, 0xffff, PNG_FILLER_AFTER);
        }

        // PNG_ALPHA_PNG: The RGB values are not premultiplied with the alpha value.
        //                But the RGB values are gamma encoded.
        // PNG_DEFAULT_sRGB: First set the default "gamma" value to sRGB in case
        //                   the file does not have a gAMA chunk.
        png_set_alpha_mode(png_ptr, PNG_ALPHA_PNG, PNG_DEFAULT_sRGB);
        // PNG_GAMMA_LINEAR: Convert RGB values to linear space.
        png_set_alpha_mode(png_ptr, PNG_ALPHA_PNG, PNG_GAMMA_LINEAR);


        assert(self->_delegate != nullptr);
        self->_delegate->info(self);

        png_read_update_info(png_ptr, info);

        // Check if the rowbytes is correct: RRGGBBAA, 8 bytes.
        if (png_get_rowbytes(png_ptr, info) != self->_width * 8) {
            png_error(png_ptr, "PNG: rowbytes != width * 8");
        }

        png_start_read_image(png_ptr);
    }

    static void _row_callback(png_structp png_ptr, png_bytep row, png_uint_32 row_nr, int pass)
    {
        auto const _self = png_get_progressive_ptr(png_ptr);
        assert(_self != nullptr);
        auto const self = std::launder(static_cast<png_loader *>(_self));

        assert(self->_delegate != nullptr);
        //self->_delegate->row(self, , row_nr, pass);
    }

    static void _end_callback(png_structp png_ptr, png_infop info) noexcept
    {
        auto const _self = png_get_progressive_ptr(png_ptr);
        assert(_self != nullptr);
        auto const self = std::launder(static_cast<png_loader *>(_self));

        assert(self->_delegate != nullptr);
        self->_delegate->end(self);
    }

    [[noreturn]] static void _user_error_fn(png_structp png_ptr, png_const_charp msg) noexcept
    {
        auto const _self = png_get_error_ptr(png_ptr);
        assert(_self != nullptr);
        auto const self = std::launder(static_cast<png_loader *>(_self));

        self->_last_error_message = msg;
        if (self->_delegate) {
            self->_delegate->user_error(self, self->_last_error_message);
        }
        std::longjmp(png_jmpbuf(png_ptr), 1);
    }

    static void _user_warning_fn(png_structp png_ptr, png_const_charp msg) noexcept
    {
        auto const _self = png_get_error_ptr(png_ptr);
        assert(_self != nullptr);
        auto const self = std::launder(static_cast<png_loader *>(_self));

        if (self->_delegate) {
            self->_delegate->user_warning(self, msg);
        }
    }

    void _cleanup() noexcept
    {
        _delegate = nullptr;
        if (std::setjmp(png_jmpbuf(_png_ptr))) {
            // At this point if something goes wrong, it must be really bad.
            std::abort();
        }

        if (_info_ptr) {
            png_destroy_info_struct(_png_ptr, &_info_ptr);
        }

        if (_png_ptr) {
            png_destroy_read_struct(&_png_ptr, nullptr, nullptr);
        }
    }
};

[[nodiscard]] pixmap<sfloat_rgba16> load_png(png_loader_delegate &delegate);

[[nodiscard]] pixmap<sfloat_rgba16> load_png(std::function<void(std::span<std::byte>)> read_function);

[[nodiscard]] pixmap<sfloat_rgba16> load_png(std::filesystem::path path);

}

hi_warning_pop();
