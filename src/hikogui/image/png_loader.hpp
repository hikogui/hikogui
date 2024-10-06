
#pragma once

#include "../macros.hpp"
#include <png/png.h>

hi_export_module(hikogui.image : png_loader);

hi_export namespace hi::inline v1 {

class png_interlace_method {
    none
    adam7
};

class png_loader;

class png_loader_delegate {
public:
    virtual std::error_code start(png_loader *sender, load_png_info& info) noexcept = 0;

    virtual std::error_code row(png_loader *sender, load_png_info& info, std::span<sfloat_rgba16> row, size_t row_nr, unsigned int pass) noexcept = 0;

    virtual std::error_code finish(png_loader *sender, load_png_info& info) noexcept = 0;

    void user_error(png_loader *sender, std::string_view msg) noexcept = 0;
    void user_warning(png_loader *sender, std::string_view msg) noexcept = 0;
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

        if (std::setjmp(png_jmpbuf(_png_ptr)) {
            throw std::runtime_error(_last_error_message);
        }

        _info_ptr = png_create_info_struct(png_ptr);
        if (_info_ptr == nullptr) {
            _cleanup();
            throw std::runtime_error("png_create_info_struct 1");
        }

        png_set_progressive_read_fn(_png_ptr, this, _info_callback, _row_callback, _end_callback);
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
        if (std::setjmp(png_jmpbuf(_png_ptr)) {
            throw std::runtime_error(_last_error_message);
        }

        png_set_sig_bytes(_png_ptr, nr_bytes);
    }

    void add_data(std::span<std::byte const> data)
    {
        if (std::setjmp(png_jmpbuf(_png_ptr)) {
            throw std::runtime_error(_last_error_message);
        }

        png_process_data(_png_ptr, _info_ptr, data.data(), data.size());
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
    [[nodiscard]] png_interlace_method interlace_method() const
    {
        switch (_interlace_method) {
        case PNG_INTERLACE_NONE:
            return png_interlace_method::none;
        case PNG_INTERLACE_ADAM7:
            return png_interlace_method::adam7;
        default:
            throw std::runtime_error(std::format("PNG: unknown interlace method: {}", _interlace_method));
        }
    }

private:
    png_loader_delegate *_delegate = nullptr;
    png_structp _png_ptr = nullptr;
    png_infop _info_ptr = nullptr;
    std::string _last_error_message = {};
    png_uint_32 _width = 0;
    png_uint_32 _height = 0;
    int _bit_depth = 0;
    int _color_type = 0;
    int _interlace_method = 0;
    int _compression_method = 0;
    int _filter_method = 0;

    pixmap<> _interleave_image;

    static void _info_callback(png_structp png_ptr, png_infop info) noexcept
    {
        auto const _self = png_get_progressive_ptr(png_ptr);
        assert(_self != nullptr);
        auto const self = std::launder(static_cast<png_loader *>(_self));

        png_get_IHDR(
            png_ptr,
            info,
            &self->_width,
            &self->_height,
            &self->_bit_depth,
            &self->_color_type,
            &self->_interlace_method,
            &self->_filter_method)


        assert(self->delegate != nullptr);
        self->delegate->info(self);

        if() {
            png_read_update_info(png_ptr, info_ptr);
        } else {
            png_start_read_image(png_ptr);
        }
    }

    static void _row_callback(png_structp png_str, png_bytep row, png_uint_32 row_nr, int pass)
    {
        auto const _self = png_get_progressive_ptr(png_ptr);
        assert(_self != nullptr);
        auto const self = std::launder(static_cast<png_loader *>(_self));

        assert(self->delegate != nullptr);
        self->delegate->row(self, , row_nr, pass);
    }

    static void _end_callback(png_structp png_ptr, png_infop info) noexcept
    {
        auto const _self = png_get_progressive_ptr(png_ptr);
        assert(_self != nullptr);
        auto const self = std::launder(static_cast<png_loader *>(_self));

        assert(self->delegate != nullptr);
        self->delegate->end(self);
    }

    static void _user_error_fn(png_structp png_ptr, png_const_charp msg) noexcept
    {
        auto const _self = png_get_error_ptr(png_ptr);
        assert(_self != nullptr);
        auto const self = std::launder(static_cast<png_loader *>(_self));

        _last_error_message = msg;
        if (delegate) {
            delegate->user_error(self, _last_error_message);
        }
        std::longjmp(png_ptr->jmpbuf, 1);
    }

    static void _user_warning_fn(png_structp png_ptr, png_const_charp msg) noexcept
    {
        auto const _self = png_get_error_ptr(png_ptr);
        assert(_self != nullptr);
        auto const self = std::launder(static_cast<png_loader *>(_self));

        if (delegate) {
            delegate->user_warning(self, msg);
        }
    }

    void _cleanup() noexcept
    {
        _delegate = nullptr;
        if (std::setjmp(png_jmpbuf(_png_ptr)) {
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

[[nodiscard]] pixmap<sfloat_rgba16> load_png(load_png_delegate &delegate)
{
}

[[nodiscard]] pixmap<sfloat_rgba16> load_png(std::function<void(std::span<std::byte>)> read_function);

[[nodiscard]] pixmap<sfloat_rgba16> load_png(std::filesystem::path path);

}

