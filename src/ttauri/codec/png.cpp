// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "png.hpp"
#include "zlib.hpp"
#include "../endian.hpp"
#include "../placement.hpp"
#include "../color/sRGB.hpp"
#include "../color/Rec2100.hpp"
#include "../color/color_space.hpp"

namespace tt {

struct PNGHeader {
    uint8_t signature[8];
};

struct ChunkHeader {
    big_uint32_buf_t length;    
    uint8_t type[4];    
};


struct IHDR {
    big_uint32_buf_t width;    
    big_uint32_buf_t height;    
    uint8_t bit_depth;
    uint8_t color_type;
    uint8_t compression_method;
    uint8_t filter_method;
    uint8_t interlace_method;
};

struct gAMA {
    big_uint32_buf_t gamma;    
};

struct cHRM {
    big_uint32_buf_t white_point_x;    
    big_uint32_buf_t white_point_y;    
    big_uint32_buf_t red_x;    
    big_uint32_buf_t red_y;    
    big_uint32_buf_t green_x;    
    big_uint32_buf_t green_y;    
    big_uint32_buf_t blue_x;    
    big_uint32_buf_t blue_y;    
};

struct sRGB {
    uint8_t rendering_intent;
};

void png::read_header(std::span<std::byte const> bytes, ssize_t &offset)
{
    ttlet png_header = make_placement_ptr<PNGHeader>(bytes, offset);

    ttlet valid_signature =
        png_header->signature[0] == 137 &&
        png_header->signature[1] == 80 &&
        png_header->signature[2] == 78 &&
        png_header->signature[3] == 71 &&
        png_header->signature[4] == 13 &&
        png_header->signature[5] == 10 &&
        png_header->signature[6] == 26 &&
        png_header->signature[7] == 10;

    tt_parse_check(valid_signature, "invalid PNG file signature");
}

void png::generate_sRGB_transfer_function() noexcept
{
    ttlet value_range = _bit_depth == 8 ? 256 : 65536;
    ttlet value_range_f = narrow_cast<float>(value_range);
    for (int i = 0; i != value_range; ++i) {
        auto u = narrow_cast<float>(i) / value_range_f;
        _transfer_function.push_back(sRGB_gamma_to_linear(u));
    }
}

void png::generate_Rec2100_transfer_function() noexcept
{
    // SDR brightness is 80 cd/m2. Rec2100/PQ brightness is 10,000 cd/m2.
    constexpr float hdr_multiplier = 10'000.0f / 80.0f;

    ttlet value_range = _bit_depth == 8 ? 256 : 65536;
    ttlet value_range_f = narrow_cast<float>(value_range);
    for (int i = 0; i != value_range; ++i) {
        auto u = narrow_cast<float>(i) / value_range_f;
        _transfer_function.push_back(Rec2100_gamma_to_linear(u) * hdr_multiplier);
    }
}

void png::generate_gamma_transfer_function(float gamma) noexcept
{
    ttlet value_range = _bit_depth == 8 ? 256 : 65536;
    ttlet value_range_f = narrow_cast<float>(value_range);
    for (int i = 0; i != value_range; ++i) {
        auto u = narrow_cast<float>(i) / value_range_f;
        _transfer_function.push_back(powf(u, gamma));
    }
}


void png::read_IHDR(std::span<std::byte const> bytes)
{
    ttlet ihdr = make_placement_ptr<IHDR>(bytes);

    _width = ihdr->width.value();
    _height = ihdr->height.value();
    _bit_depth = ihdr->bit_depth;
    _color_type = ihdr->color_type;
    _compression_method = ihdr->compression_method;
    _filter_method = ihdr->filter_method;
    _interlace_method = ihdr->interlace_method;

    tt_parse_check(_width <= 16384, "PNG width too large.");
    tt_parse_check(_height <= 16384, "PNG height too large.");
    tt_parse_check(_bit_depth == 8 || _bit_depth == 16, "PNG only bit depth of 8 or 16 is implemented.");
    tt_parse_check(_compression_method == 0, "Only deflate/inflate compression is allowed.");
    tt_parse_check(_filter_method == 0, "Only adaptive filtering is allowed.");
    tt_parse_check(_interlace_method == 0, "Only non interlaced PNG are implemented.");

    _is_palletted = (_color_type & 1) != 0;
    _is_color = (_color_type & 2) != 0;
    _has_alpha = (_color_type & 4) != 0;
    tt_parse_check((_color_type & 0xf8) == 0, "Invalid color type");
    tt_parse_check(!_is_palletted, "Paletted images are not supported");

    if (_is_palletted) {
        _samples_per_pixel = 1;
    } else {
        _samples_per_pixel = static_cast<int>(_has_alpha);
        _samples_per_pixel += _is_color ? 3 : 1;
    }

    _bits_per_pixel = _samples_per_pixel * _bit_depth;
    _bytes_per_line = (_bits_per_pixel * _width + 7) / 8;
    _stride = _bytes_per_line + 1;
    _bytes_per_pixel = std::max(1, _bits_per_pixel / 8);

    generate_sRGB_transfer_function();
}

void png::read_cHRM(std::span<std::byte const> bytes)
{
    ttlet chrm = make_placement_ptr<cHRM>(bytes);

    ttlet color_to_XYZ = color_primaries_to_RGBtoXYZ(
        narrow_cast<float>(chrm->white_point_x.value()) / 100'000.0f,
        narrow_cast<float>(chrm->white_point_y.value()) / 100'000.0f,
        narrow_cast<float>(chrm->red_x.value()) / 100'000.0f,
        narrow_cast<float>(chrm->red_y.value()) / 100'000.0f,
        narrow_cast<float>(chrm->green_x.value()) / 100'000.0f,
        narrow_cast<float>(chrm->green_y.value()) / 100'000.0f,
        narrow_cast<float>(chrm->blue_x.value()) / 100'000.0f,
        narrow_cast<float>(chrm->blue_y.value()) / 100'000.0f
    );

    _color_to_sRGB = XYZ_to_sRGB * color_to_XYZ;
}

void png::read_gAMA(std::span<std::byte const> bytes)
{
    ttlet gama = make_placement_ptr<gAMA>(bytes);
    ttlet gamma = narrow_cast<float>(gama->gamma.value()) / 100'000.0f;
    tt_parse_check(gamma != 0.0f, "Gamma value can not be zero");
     
    generate_gamma_transfer_function(1.0f / gamma);
}

void png::read_sRGB(std::span<std::byte const> bytes)
{
    ttlet srgb = make_placement_ptr<sRGB>(bytes);
    ttlet rendering_intent = srgb->rendering_intent;
    tt_parse_check(rendering_intent <= 3, "Invalid rendering intent");

    _color_to_sRGB = geo::identity();
    generate_sRGB_transfer_function();
}

static std::string read_string(std::span<std::byte const> bytes)
{
    std::string r;

    for (ssize_t i = 0; i != std::ssize(bytes); ++i) {
        auto c = static_cast<char>(bytes[i]);
        if (c == 0) {
            return r;
        } else {
            r += c;
        }
    }
    throw parse_error("string is not null terminated.");
}

void png::read_iCCP(std::span<std::byte const> bytes)
{
    auto profile_name = read_string(bytes);

    if (profile_name == "ITUR_2100_PQ_FULL") {
        // The official rule here is to ignore everything in the ICC profile and
        // create the conversion matrix and transfer function from scratch.

        _color_to_sRGB = XYZ_to_sRGB * Rec2100_to_XYZ;
        generate_Rec2100_transfer_function();
        return;
    }
}

void png::read_chunks(std::span<std::byte const> bytes, ssize_t &offset)
{
    auto IHDR_bytes = std::span<std::byte const>{};
    auto cHRM_bytes = std::span<std::byte const>{};
    auto gAMA_bytes = std::span<std::byte const>{};
    auto iCCP_bytes = std::span<std::byte const>{};
    auto sRGB_bytes = std::span<std::byte const>{};
    bool has_IEND = false;

    while (!has_IEND) {
        ttlet header = make_placement_ptr<ChunkHeader>(bytes, offset);
        ttlet length = narrow_cast<ssize_t>(header->length.value());
        tt_parse_check(length < 0x8000'0000, "Chunk length must be smaller than 2GB");
        tt_parse_check(offset + length + ssizeof(uint32_t) <= std::ssize(bytes), "Chuck extents beyond file.");

        switch (fourcc(header->type)) {
        case fourcc("IDAT"):
            _idat_chunk_data.push_back(bytes.subspan(offset, length));
            break;

        case fourcc("IHDR"):
            IHDR_bytes = bytes.subspan(offset, length);
            break;

        case fourcc("cHRM"):
            cHRM_bytes = bytes.subspan(offset, length);
            break;

        case fourcc("gAMA"):
            gAMA_bytes = bytes.subspan(offset, length);
            break;

        case fourcc("iCCP"):
            iCCP_bytes = bytes.subspan(offset, length);
            break;

        case fourcc("sRGB"):
            sRGB_bytes = bytes.subspan(offset, length);
            break;

        case fourcc("IEND"):
            has_IEND = true;
            break;

        default:;
        }

        // Skip over the data, and extract the crc32.
        offset += length;
        [[maybe_unused]] ttlet crc = make_placement_ptr<big_uint32_buf_t>(bytes, offset);
    }

    tt_parse_check(!IHDR_bytes.empty(), "Missing IHDR chunk.");
    read_IHDR(IHDR_bytes);
    if (!cHRM_bytes.empty()) {
        read_cHRM(cHRM_bytes);
    }
    if (!gAMA_bytes.empty()) {
        read_gAMA(gAMA_bytes);
    }

    // Will override cHRM and gAMA chunks.
    if (!iCCP_bytes.empty()) {
        read_iCCP(iCCP_bytes);
    }

    // Will override cHRM, gAMA and ICCP chunks.
    if (!sRGB_bytes.empty()) {
        read_sRGB(sRGB_bytes);
    }

}

png::png(std::span<std::byte const> bytes) :
    _view()
{
    ssize_t offset = 0;

    read_header(bytes, offset);
    read_chunks(bytes, offset);
}

png::png(std::unique_ptr<resource_view> view) :
    _view(std::move(view))
{
    ssize_t offset = 0;

    ttlet bytes = _view->bytes();
    read_header(bytes, offset);
    read_chunks(bytes, offset);
}

bstring png::decompress_IDATs(ssize_t image_data_size) const {
    if (std::ssize(_idat_chunk_data) == 1) {
        return zlib_decompress(_idat_chunk_data[0], image_data_size);
    } else {
        // Merge all idat chunks together.
        ttlet compressed_data_size = std::accumulate(
            _idat_chunk_data.cbegin(), _idat_chunk_data.cend(), ssize_t{0},
            [](ttlet &a, ttlet &b) {
            return a + std::ssize(b);
        }
        );

        bstring compressed_data;
        compressed_data.reserve(compressed_data_size);
        for (ttlet &chunk_data : _idat_chunk_data) {
            std::copy(chunk_data.begin(), chunk_data.end(), std::back_inserter(compressed_data));
        }

        return zlib_decompress(compressed_data, image_data_size);
    }
}

void png::unfilter_line_sub(std::span<uint8_t> line, std::span<uint8_t const> prev_line) const noexcept
{
    for (int i = 0; i != _bytes_per_line; ++i) {
        int j = i - _bytes_per_pixel;

        uint8_t prev_raw = j >= 0 ? line[j] : 0;
        line[i] += prev_raw;
    }
}

void png::unfilter_line_up(std::span<uint8_t> line, std::span<uint8_t const> prev_line) const noexcept
{
    for (int i = 0; i != _bytes_per_line; ++i) {
        line[i] += prev_line[i];
    }
}

void png::unfilter_line_average(std::span<uint8_t> line, std::span<uint8_t const> prev_line) const noexcept
{
    for (int i = 0; i != _bytes_per_line; ++i) {
        int j = i - _bytes_per_pixel;

        uint8_t prev_raw = j >= 0 ? line[j] : 0;
        line[i] += (prev_raw + prev_line[i]) / 2;
    }
}

static uint8_t paeth_predictor(uint8_t _a, uint8_t _b, uint8_t _c) noexcept {
    auto a = static_cast<int>(_a);
    auto b = static_cast<int>(_b);
    auto c = static_cast<int>(_c);

    auto p = a + b - c;
    auto pa = std::abs(p - a);
    auto pb = std::abs(p - b);
    auto pc = std::abs(p - c);

    if (pa <= pb && pa <= pc) {
        return static_cast<uint8_t>(a);
    } else if (pb <= pc) {
        return static_cast<uint8_t>(b);
    } else {
        return static_cast<uint8_t>(c);
    }
}

void png::unfilter_line_paeth(std::span<uint8_t> line, std::span<uint8_t const> prev_line) const noexcept
{
    for (int i = 0; i != _bytes_per_line; ++i) {
        int j = i - _bytes_per_pixel;

        uint8_t up = prev_line[i];
        uint8_t left = j >= 0 ? line[j] : 0;
        uint8_t left_up = j >= 0 ? prev_line[j] : 0;
        line[i] += paeth_predictor(left, up, left_up);
    }
}

void png::unfilter_line(std::span<uint8_t> line, std::span<uint8_t const> prev_line) const
{
    switch (line[0]) {
    case 0: return;
    case 1: return unfilter_line_sub(line.subspan(1, _bytes_per_line), prev_line);
    case 2: return unfilter_line_up(line.subspan(1, _bytes_per_line), prev_line);
    case 3: return unfilter_line_average(line.subspan(1, _bytes_per_line), prev_line);
    case 4: return unfilter_line_paeth(line.subspan(1, _bytes_per_line), prev_line);
    default:
        throw parse_error("Unknown line-filter type");
    }
}

void png::unfilter_lines(bstring &image_data) const
{
    auto image_bytes = std::span(reinterpret_cast<uint8_t *>(image_data.data()), std::ssize(image_data));
    auto zero_line = bstring(_bytes_per_line, std::byte{0});

    auto prev_line = std::span(reinterpret_cast<uint8_t *>(zero_line.data()), std::ssize(zero_line));
    for (int y = 0; y != _height; ++y) {
        auto line = image_bytes.subspan(y * _stride, _stride);
        unfilter_line(line, prev_line);
        prev_line = line.subspan(1, _bytes_per_line);
    }
}

static uint16_t get_sample(std::span<std::byte const> bytes, ssize_t &offset, bool two_bytes)
{
    uint16_t value = static_cast<uint8_t>(bytes[offset++]);
    if (two_bytes) {
        value <<= 8;
        value |= static_cast<uint8_t>(bytes[offset++]);
    }
    return value;
}

u16x4 png::extract_pixel_from_line(std::span<std::byte const> bytes, int x) const noexcept
{
    tt_axiom(_bit_depth == 8 || _bit_depth == 16);
    tt_axiom(!_is_palletted);

    uint16_t r = 0;
    uint16_t g = 0;
    uint16_t b = 0;
    uint16_t a = 0;

    ssize_t offset = x * _bytes_per_pixel;
    if (_is_color) {
        r = get_sample(bytes, offset, _bit_depth == 16);
        g = get_sample(bytes, offset, _bit_depth == 16);
        b = get_sample(bytes, offset, _bit_depth == 16);
    } else {
        r = g = b = get_sample(bytes, offset, _bit_depth == 16);
    }
    if (_has_alpha) {
        a = get_sample(bytes, offset, _bit_depth == 16);
    } else {
        a = (_bit_depth == 16) ? 65535 : 255;
    }

    return {r, g, b, a};
}

void png::data_to_image_line(std::span<std::byte const> bytes, pixel_row<sfloat_rgba16> &line) const noexcept
{
    ttlet alpha_mul = _bit_depth == 16 ? 1.0f/65535.0f : 1.0f/255.0f;
    for (int x = 0; x != _width; ++x) {
        ttlet value = extract_pixel_from_line(bytes, x);

        ttlet linear_color = color(
            _transfer_function[value.x()],
            _transfer_function[value.y()],
            _transfer_function[value.z()]
        );

        auto lesRGB_color = _color_to_sRGB * linear_color;
        lesRGB_color.a() = static_cast<float>(value.w()) * alpha_mul;

        line[x] = lesRGB_color;
    }
}

void png::data_to_image(bstring bytes, pixel_map<sfloat_rgba16> &image) const noexcept
{
    auto bytes_span = std::span(bytes);

    for (int y = 0; y != _height; ++y) {
        int inv_y = _height - y - 1;

        auto bytes_line = bytes_span.subspan(inv_y * _stride + 1, _bytes_per_line);
        auto pixel_line = image[y];
        data_to_image_line(bytes_line, pixel_line);
    }
}

void png::decode_image(pixel_map<sfloat_rgba16> &image) const
{
    // There is a filter selection byte in front of every line.
    ttlet image_data_size = _stride * _height;

    auto image_data = decompress_IDATs(image_data_size);
    tt_parse_check(std::ssize(image_data) == image_data_size, "Uncompressed image data has incorrect size.");

    unfilter_lines(image_data);

    data_to_image(image_data, image);

}

pixel_map<sfloat_rgba16> png::load(URL const &url)
{
    ttlet png_data = png(url);
    auto image = pixel_map<sfloat_rgba16>{narrow_cast<ssize_t>(png_data.width()), narrow_cast<ssize_t>(png_data.height())};
    png_data.decode_image(image);
    return image;
}


}
