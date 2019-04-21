
#include "PNG.hpp"

#include <png.h>
#include <stdio.h>

namespace TTauri::Draw {

using namespace std;

PixelMap<uint32_t> loadPNG(const PixelMap<uint32_t> &pixelMap, const std::filesystem::path &path)
{
    // XXX Replace with memory mapped IO.
    string stringPath = path.string();

    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;
    png_infop end_info = nullptr;
    FILE *fp = nullptr;

#define PNG_THROW_EXCEPTION(e) \
    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info); \
    fclose(fp); \
    BOOST_THROW_EXCEPTION(e);

    if (!(fp = fopen(stringPath.data(), "rb"))) {
        PNG_THROW_EXCEPTION(PNGFileOpenError());
    }

    size_t const PNGHeader_size = 8;
    png_byte PNGHeader[PNGHeader_size] = {0, 0, 0, 0, 0, 0, 0, 0};
    if (fread(PNGHeader, sizeof(*PNGHeader), PNGHeader_size, fp) != PNGHeader_size) {
        PNG_THROW_EXCEPTION(PNGReadError());
    }

    if (png_sig_cmp(PNGHeader, 0, PNGHeader_size) != 0) {
        PNG_THROW_EXCEPTION(PNGHeaderError());
    }

    if (!(png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr))) {
        PNG_THROW_EXCEPTION(PNGInitializationError());
    }

    if (!(info_ptr = png_create_info_struct(png_ptr))) {
        PNG_THROW_EXCEPTION(PNGInitializationError());
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        PNG_THROW_EXCEPTION(PNGParseError());
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, PNGHeader_size);
    png_set_user_limits(png_ptr, boost::numeric_cast<png_uint_32>(pixelMap.width), boost::numeric_cast<png_uint_32>(pixelMap.height));
    png_set_alpha_mode(png_ptr, PNG_ALPHA_PNG, PNG_DEFAULT_sRGB);
    
    auto row_pointers = pixelMap.rowPointers();
    png_bytepp row_pointers_data = reinterpret_cast<png_bytepp>(row_pointers.data());
    png_set_rows(png_ptr, info_ptr, row_pointers_data);

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_SCALE_16 | PNG_TRANSFORM_GRAY_TO_RGB, NULL);

    auto const width = png_get_image_width(png_ptr, info_ptr);
    auto const height = png_get_image_height(png_ptr, info_ptr);

    png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
    fclose(fp);

    return pixelMap.submap(0, 0, width, height);
}

}