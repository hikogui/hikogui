// Copyright 2019 Pokitec
// All rights reserved.

#include "PNG.hpp"
#include "PixelMap.hpp"
#include "TTauri/logger.hpp"
#include "TTauri/wsRGBA.hpp"
#include "TTauri/required.hpp"
#include "TTauri/URL.hpp"
#include <png.h>
#include <stdio.h>

namespace TTauri::Draw {

using namespace std;

PixelMap<wsRGBA> loadPNG(PixelMap<wsRGBA> &pixelMap, const URL &path)
{
    // XXX Replace with memory mapped IO.
    string stringPath = path.path_string();

    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;
    png_infop end_info = nullptr;
    FILE *fp = nullptr;

#define PNG_THROW_EXCEPTION(e) \
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info); \
    if (fp != nullptr) { fclose(fp); } \
    TTAURI_THROW(e);

    fp = fopen(stringPath.data(), "rb");
    if (fp == nullptr) {
        PNG_THROW_EXCEPTION(io_error("Could not open .png file")
            << error_info<"url"_tag>(path)
        );
    }

    size_t const PNGHeader_size = 8;
    png_byte PNGHeader[PNGHeader_size] = {0, 0, 0, 0, 0, 0, 0, 0};
    if (fread(PNGHeader, sizeof(*PNGHeader), PNGHeader_size, fp) != PNGHeader_size) {
        PNG_THROW_EXCEPTION(io_error("Could not read .png file")
            << error_info<"url"_tag>(path)
        );
    }

    if (png_sig_cmp(PNGHeader, 0, PNGHeader_size) != 0) {
        PNG_THROW_EXCEPTION(parse_error("Could not parse .png header")
            << error_info<"url"_tag>(path)
        );
    }

    if (!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr))) {
        LOG_FATAL("Call to png_create_read_struct() failed.");
    }

    if (!(info_ptr = png_create_info_struct(png_ptr))) {
        LOG_FATAL("Call to png_create_info_struct() failed.");
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        PNG_THROW_EXCEPTION(parse_error("Could not parse .png header")
            << error_info<"url"_tag>(path)
        );
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, PNGHeader_size);
    png_set_user_limits(png_ptr, numeric_cast<png_uint_32>(pixelMap.width), numeric_cast<png_uint_32>(pixelMap.height));

    png_color_8 sig_bit;
    sig_bit.red = 16;
    sig_bit.green = 16;
    sig_bit.blue = 16;
    sig_bit.alpha = 16;
    png_set_sBIT(png_ptr, info_ptr, &sig_bit);
    png_set_expand_16(png_ptr);
    // First set the default when the color space was not set in the png file.
    png_set_alpha_mode(png_ptr, PNG_ALPHA_PNG, PNG_DEFAULT_sRGB);
    // Now override with how we want to to look.
    png_set_alpha_mode(png_ptr, PNG_ALPHA_PREMULTIPLIED, PNG_GAMMA_LINEAR);

    auto row_pointers = pixelMap.rowPointers();
    std::reverse(row_pointers.begin(), row_pointers.end());
    png_bytepp row_pointers_data = reinterpret_cast<png_bytepp>(row_pointers.data());
    png_set_rows(png_ptr, info_ptr, row_pointers_data);

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND_16 | PNG_TRANSFORM_GRAY_TO_RGB | PNG_TRANSFORM_SWAP_ENDIAN, NULL);

    let width = png_get_image_width(png_ptr, info_ptr);
    let height = png_get_image_height(png_ptr, info_ptr);

    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

    fclose(fp);

    return pixelMap.submap(0, 0, width, height);
}

}
