
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

enum class png_iccp_profile {
    unknown,
    ITUR_2100_PQ_FULL
};

class png_loader;

class png_loader_delegate {
public:
    static void adam7_sparkle(
        std::span<sfloat_rgba16> src,
        unsigned int pass,
        size_t width,
        std::span<sfloat_rgba16> dst,
        std::span<sfloat_rgba16> d1,
        std::span<sfloat_rgba16> d2,
        std::span<sfloat_rgba16> d3,
        std::span<sfloat_rgba16> d4,
        std::span<sfloat_rgba16> d5,
        std::span<sfloat_rgba16> d6,
        std::span<sfloat_rgba16> d7)
    {

    }


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

    png_loader(png_loader_delegate &delegate) : _delegate(std::addressof(delegate))
    {
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

    struct interlace_info_type {
        uint8_t x_stride;
        uint8_t y_stride;
        uint8_t x_offset;
        uint8_t y_offset;
    };

    [[nodiscard]] interlace_info_type interlace_info(unsigned int pass) const
    {
        switch (interlace_method()) {
        case png_interlace_method::none:
            assert(pass == 0);
            return {1, 1, 0, 0};

        case png_interlace_method::adam7:
            return {
                uint8_t{1} << PNG_PASS_COL_SHIFT(pass),
                uint8_t{1} << PNG_PASS_ROW_SHIFT(pass),
                PNG_PASS_START_COL(pass),
                PNG_PASS_START_ROW(pass)
            };
        }
        std::unreachable();
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
    std::optional<color_primaries> _color_primaries = {}
    png_iccp_profile _iccp_profile = png_iccp_profile::unknown;
    std::vector<sfloat_rgba16> _row;

    [[nodiscard]] static png_interlace_method _png_get_interlace_type(png_structp png_ptr, png_infop info) noexcept
    {
        switch (png_get_interlace_type(png_ptr, info)) {
        case PNG_INTERLACE_NONE:
            return png_interlace_method::none;
        case PNG_INTERLACE_ADAM7:
            return png_interlace_method::adam7;
        default:
            png_error(png_ptr, "PNG: unknown interlace method");
        }
        std::unreachable();
    }

    [[nodiscard]] static png_color_type _png_get_color_type(png_structp png_ptr, png_infop info) noexcept
    {
        switch (png_get_color_type(png_ptr, info)) {
        case PNG_COLOR_TYPE_GRAY:
            return png_color_type::gray;
        case PNG_COLOR_TYPE_GRAY_ALPHA:
            return png_color_type::gray_alpha;
        case PNG_COLOR_TYPE_PALETTE:
            return png_color_type::palette;
        case PNG_COLOR_TYPE_RGB:
            return png_color_type::rgb;
        case PNG_COLOR_TYPE_RGB_ALPHA:
            return png_color_type::rgb_alpha;
        default:
            png_error(png_ptr, "PNG: unknown color type");
        }
        std::unreachalbe();
    }

    [[nodiscard]] static std::optional<color_primaries> _png_get_cHRM(png_structp png_ptr, png_infop info) noexcept
    {
        double wx = 0.0;
        double wy = 0.0;
        double rx = 0.0;
        double ry = 0.0;
        double gx = 0.0;
        double gy = 0.0;
        double bx = 0.0;
        double by = 0.0;

        if (png_get_cHRM(png_ptr, info, &wx, &wy, &rx, &ry, &gx, &gy, &bx, &by)) {
            return color_primaries{
                gsl::narrow<float>(wx), 
                gsl::narrow<float>(wy), 
                gsl::narrow<float>(rx), 
                gsl::narrow<float>(ry), 
                gsl::narrow<float>(gx), 
                gsl::narrow<float>(gy), 
                gsl::narrow<float>(bx), 
                gsl::narrow<float>(by)};
        }
        
        return std::nullopt;
    }

    [[nodiscard]] static png_iccp_profile _png_get_iCCP(png_structp png_ptr, png_infop info) noexcept
    {
        png_charp name = nullptr;
        int compression_type = 0;;
        png_charp profile = nullptr;
        png_uint_32 profle_size = 0;

        if (png_get_iCCP(png_ptr, info, &name, &compression_type, &profile, &profile_size)) {
            auto const _name = std::string_view{name};

            if (_name == "ITUR_2100_PQ_FULL") {
                return png_iccp_profile::ITUR_2100_PQ_FULL;
            }
        }

        return png_iccp_profile::unknown;
    }

    static void _info_callback(png_structp png_ptr, png_infop info) noexcept
    {
        auto const self = std::launder(static_cast<png_loader *>(png_get_progressive_ptr(png_ptr)));
        if (self == nullptr or self->delegate == nullptr) {
            png_error(png_ptr, "_info_callback failed preconditions");
        }

        self->_width = png_get_image_width(png_ptr, info);
        self->_height = png_get_image_height(png_ptr, info);
        self->_bit_depth = png_get_bit_depth(png_ptr, info);
        self->_interlace_method = _png_get_interlace_type(png_ptr, info);
        self->_color_type = _png_get_color_type(png_ptr, info);
        self->_color_primaries = _png_get_cHRM(png_ptr, info);
        self->_iccp_profile = _png_get_iCCP(png_ptr, info);

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
        // This is needed to have enough precision for conversion to linear space.
        if (self->_bit_depth < 16) {
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

        // Calculate the optional color conversion matrix.
        if (self->_color_primaries) {
            auto const image_to_XYZ = color_primaries_to_RGBtoXYZ(*self->_color_primaries);
            self->_color_matrix = XYZ_to_sRGB * image_to_XYZ;
        } else {
            self->_color_matrix = std::nullopt;
        }

        // Calculate how to convert 16-bit color components to float.
        switch (self->_iccp_profile) {
        case png_iccp_profile::unknown:
            self->_component_scale = 1.0f / 65535.0f;
            break;
        case png_iccp_profile::ITUR_2100_PQ_FULL:
            // HDR full scale luminosity is 1000 cd/m2.
            self->_component_scale = 12.5f / 65535.0f;
            break;
        }

        self->_delegate->info(self);

        // Check if the rowbytes is correct: RRGGBBAA, 8 bytes.
        png_read_update_info(png_ptr, info);
        if (png_get_rowbytes(png_ptr, info) != self->_width * 8) {
            png_error(png_ptr, "PNG: rowbytes != width * 8");
        }

        // Reserve the row to pass to the delegate later on.
        _row.resize(self->_width);
    }

    static void _row_callback(png_structp png_ptr, png_bytep row, png_uint_32 row_nr, int pass)
    {
        auto const self = std::launder(static_cast<png_loader *>(png_get_progressive_ptr(png_ptr)));
        if (self == nullptr or self->delegate == nullptr) {
            png_error(png_ptr, "_row_callback failed preconditions");
        }

        auto const ii = self->interlace_info(pass);
        auto const num_columns = self->width() / ii.x_stride;
        auto const out = std::span{self->_row.data(), num_columns};

        auto const c_scale = self->_component_scale;
        constexpr auto a_scale = 1.0f / 65535.0f;

        if (self->_color_matrix) {
            auto const M = *self->_color_matrix;
            auto off = size_t{0};
            for (auto x = size_t{0}; x != num_columns; ++x, off += 8) {
                auto const RR = (static_cast<uint16_t>(row[off + 0]) << 8) | static_cast<uint16_t>(row[off + 1]);
                auto const GG = (static_cast<uint16_t>(row[off + 2]) << 8) | static_cast<uint16_t>(row[off + 3]);
                auto const BB = (static_cast<uint16_t>(row[off + 4]) << 8) | static_cast<uint16_t>(row[off + 5]);
                auto const AA = (static_cast<uint16_t>(row[off + 6]) << 8) | static_cast<uint16_t>(row[off + 7]);

                out[x] = M * color{RR * c_scale, GG * c_scale, BB * c_scale, AA * a_scale};
            }

        } else {
            auto off = size_t{0};
            for (auto x = size_t{0}; x != num_columns; ++x, off += 8) {
                auto const RR = (static_cast<uint16_t>(row[off + 0]) << 8) | static_cast<uint16_t>(row[off + 1]);
                auto const GG = (static_cast<uint16_t>(row[off + 2]) << 8) | static_cast<uint16_t>(row[off + 3]);
                auto const BB = (static_cast<uint16_t>(row[off + 4]) << 8) | static_cast<uint16_t>(row[off + 5]);
                auto const AA = (static_cast<uint16_t>(row[off + 6]) << 8) | static_cast<uint16_t>(row[off + 7]);

                out[x] = color{RR * c_scale, GG * c_scale, BB * c_scale, AA * a_scale};
            }
        }

        self->_delegate->row(self, out, row_nr, pass);
    }

    static void _end_callback(png_structp png_ptr, png_infop info) noexcept
    {
        auto const self = std::launder(static_cast<png_loader *>(png_get_progressive_ptr(png_ptr)));
        if (self == nullptr or self->delegate == nullptr) {
            png_error(png_ptr, "_end_callback failed preconditions");
        }

        self->_delegate->end(self);
    }

    [[noreturn]] static void _user_error_fn(png_structp png_ptr, png_const_charp msg) noexcept
    {
        auto const self = std::launder(static_cast<png_loader *>(png_get_error_ptr(png_ptr)));
        assert(self != nullptr);

        self->_last_error_message = msg;
        if (self->_delegate) {
            self->_delegate->user_error(self, self->_last_error_message);
        }
        std::longjmp(png_jmpbuf(png_ptr), 1);
    }

    static void _user_warning_fn(png_structp png_ptr, png_const_charp msg) noexcept
    {
        auto const self = std::launder(static_cast<png_loader *>(png_get_error_ptr(png_ptr)));
        assert(self != nullptr);

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
