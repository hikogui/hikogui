// Copyright Jens A. Koch 2021.
// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "png.hpp"
#include "zlib.hpp"
#include "../endian.hpp"
#include "../placement.hpp"
#include "../color/sRGB.hpp"
#include "../color/Rec2100.hpp"
#include "../color/color_space.hpp"

namespace hi::inline v1 {

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

void png::read_header(std::span<std::byte const> bytes, std::size_t &offset)
{
    hilet png_header = make_placement_ptr<PNGHeader>(bytes, offset);

    hilet valid_signature = png_header->signature[0] == 137 && png_header->signature[1] == 80 && png_header->signature[2] == 78 &&
        png_header->signature[3] == 71 && png_header->signature[4] == 13 && png_header->signature[5] == 10 &&
        png_header->signature[6] == 26 && png_header->signature[7] == 10;

    hi_parse_check(valid_signature, "invalid PNG file signature");
}

void png::generate_sRGB_transfer_function() noexcept
{
    hilet value_range = _bit_depth == 8 ? 256 : 65536;
    hilet value_range_f = narrow_cast<float>(value_range);
    for (int i = 0; i != value_range; ++i) {
        auto u = narrow_cast<float>(i) / value_range_f;
        _transfer_function.push_back(sRGB_gamma_to_linear(u));
    }
}

void png::generate_Rec2100_transfer_function() noexcept
{
    // SDR brightness is 80 cd/m2. Rec2100/PQ brightness is 10,000 cd/m2.
    constexpr float hdr_multiplier = 10'000.0f / 80.0f;

    hilet value_range = _bit_depth == 8 ? 256 : 65536;
    hilet value_range_f = narrow_cast<float>(value_range);
    for (int i = 0; i != value_range; ++i) {
        auto u = narrow_cast<float>(i) / value_range_f;
        _transfer_function.push_back(Rec2100_gamma_to_linear(u) * hdr_multiplier);
    }
}

void png::generate_gamma_transfer_function(float gamma) noexcept
{
    hilet value_range = _bit_depth == 8 ? 256 : 65536;
    hilet value_range_f = narrow_cast<float>(value_range);
    for (int i = 0; i != value_range; ++i) {
        auto u = narrow_cast<float>(i) / value_range_f;
        _transfer_function.push_back(powf(u, gamma));
    }
}

void png::read_IHDR(std::span<std::byte const> bytes)
{
    hilet ihdr = make_placement_ptr<IHDR>(bytes);

    _width = ihdr->width.value();
    _height = ihdr->height.value();
    _bit_depth = ihdr->bit_depth;
    _color_type = ihdr->color_type;
    _compression_method = ihdr->compression_method;
    _filter_method = ihdr->filter_method;
    _interlace_method = ihdr->interlace_method;

    hi_parse_check(_width <= 16384, "PNG width too large.");
    hi_parse_check(_height <= 16384, "PNG height too large.");
    hi_parse_check(_bit_depth == 8 || _bit_depth == 16, "PNG only bit depth of 8 or 16 is implemented.");
    hi_parse_check(_compression_method == 0, "Only deflate/inflate compression is allowed.");
    hi_parse_check(_filter_method == 0, "Only adaptive filtering is allowed.");
    hi_parse_check(_interlace_method == 0, "Only non interlaced PNG are implemented.");

    _is_palletted = (_color_type & 1) != 0;
    _is_color = (_color_type & 2) != 0;
    _has_alpha = (_color_type & 4) != 0;
    hi_parse_check((_color_type & 0xf8) == 0, "Invalid color type");
    hi_parse_check(!_is_palletted, "Paletted images are not supported");

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
    hilet chrm = make_placement_ptr<cHRM>(bytes);

    hilet color_to_XYZ = color_primaries_to_RGBtoXYZ(
        narrow_cast<float>(chrm->white_point_x.value()) / 100'000.0f,
        narrow_cast<float>(chrm->white_point_y.value()) / 100'000.0f,
        narrow_cast<float>(chrm->red_x.value()) / 100'000.0f,
        narrow_cast<float>(chrm->red_y.value()) / 100'000.0f,
        narrow_cast<float>(chrm->green_x.value()) / 100'000.0f,
        narrow_cast<float>(chrm->green_y.value()) / 100'000.0f,
        narrow_cast<float>(chrm->blue_x.value()) / 100'000.0f,
        narrow_cast<float>(chrm->blue_y.value()) / 100'000.0f);

    _color_to_sRGB = XYZ_to_sRGB * color_to_XYZ;
}

void png::read_gAMA(std::span<std::byte const> bytes)
{
    hilet gama = make_placement_ptr<gAMA>(bytes);
    hilet gamma = narrow_cast<float>(gama->gamma.value()) / 100'000.0f;
    hi_parse_check(gamma != 0.0f, "Gamma value can not be zero");

    generate_gamma_transfer_function(1.0f / gamma);
}

void png::read_sRGB(std::span<std::byte const> bytes)
{
    hilet srgb = make_placement_ptr<sRGB>(bytes);
    hilet rendering_intent = srgb->rendering_intent;
    hi_parse_check(rendering_intent <= 3, "Invalid rendering intent");

    _color_to_sRGB = geo::identity();
    generate_sRGB_transfer_function();
}

static std::string read_string(std::span<std::byte const> bytes)
{
    std::string r;

    for (ssize_t i = 0; i != ssize(bytes); ++i) {
        hilet c = static_cast<char>(bytes[i]);
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

void png::read_chunks(std::span<std::byte const> bytes, std::size_t &offset)
{
    auto IHDR_bytes = std::span<std::byte const>{};
    auto cHRM_bytes = std::span<std::byte const>{};
    auto gAMA_bytes = std::span<std::byte const>{};
    auto iCCP_bytes = std::span<std::byte const>{};
    auto sRGB_bytes = std::span<std::byte const>{};
    bool has_IEND = false;

    while (!has_IEND) {
        hilet header = make_placement_ptr<ChunkHeader>(bytes, offset);
        hilet length = narrow_cast<ssize_t>(header->length.value());
        hi_parse_check(length < 0x8000'0000, "Chunk length must be smaller than 2GB");
        hi_parse_check(offset + length + ssizeof(uint32_t) <= bytes.size(), "Chuck extents beyond file.");

        switch (fourcc(header->type)) {
        case fourcc("IDAT"): _idat_chunk_data.push_back(bytes.subspan(offset, length)); break;

        case fourcc("IHDR"): IHDR_bytes = bytes.subspan(offset, length); break;

        case fourcc("cHRM"): cHRM_bytes = bytes.subspan(offset, length); break;

        case fourcc("gAMA"): gAMA_bytes = bytes.subspan(offset, length); break;

        case fourcc("iCCP"): iCCP_bytes = bytes.subspan(offset, length); break;

        case fourcc("sRGB"): sRGB_bytes = bytes.subspan(offset, length); break;

        case fourcc("IEND"): has_IEND = true; break;

        default:;
        }

        // Skip over the data, and extract the crc32.
        offset += length;
        [[maybe_unused]] hilet crc = make_placement_ptr<big_uint32_buf_t>(bytes, offset);
    }

    hi_parse_check(!IHDR_bytes.empty(), "Missing IHDR chunk.");
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

png::png(file_view view) : _view(std::move(view))
{
    std::size_t offset = 0;

    hilet bytes = as_bstring_view(_view);
    read_header(bytes, offset);
    read_chunks(bytes, offset);
}

bstring png::decompress_IDATs(std::size_t image_data_size) const
{
    if (ssize(_idat_chunk_data) == 1) {
        return zlib_decompress(_idat_chunk_data[0], image_data_size);
    } else {
        // Merge all idat chunks together.
        hilet compressed_data_size =
            std::accumulate(_idat_chunk_data.cbegin(), _idat_chunk_data.cend(), ssize_t{0}, [](hilet &a, hilet &b) {
                return a + ssize(b);
            });

        bstring compressed_data;
        compressed_data.reserve(compressed_data_size);
        for (hilet chunk_data : _idat_chunk_data) {
            std::copy(chunk_data.begin(), chunk_data.end(), std::back_inserter(compressed_data));
        }

        return zlib_decompress(compressed_data, image_data_size);
    }
}

void png::unfilter_line_sub(std::span<uint8_t> line, std::span<uint8_t const> prev_line) const noexcept
{
    for (int i = 0; i != _bytes_per_line; ++i) {
        hilet j = i - _bytes_per_pixel;

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
        hilet j = i - _bytes_per_pixel;

        uint8_t prev_raw = j >= 0 ? line[j] : 0;
        line[i] += (prev_raw + prev_line[i]) / 2;
    }
}

static uint8_t paeth_predictor(uint8_t _a, uint8_t _b, uint8_t _c) noexcept
{
    hilet a = static_cast<int>(_a);
    hilet b = static_cast<int>(_b);
    hilet c = static_cast<int>(_c);

    hilet p = a + b - c;
    hilet pa = std::abs(p - a);
    hilet pb = std::abs(p - b);
    hilet pc = std::abs(p - c);

    if (pa <= pb && pa <= pc) {
        return narrow_cast<uint8_t>(a);
    } else if (pb <= pc) {
        return narrow_cast<uint8_t>(b);
    } else {
        return narrow_cast<uint8_t>(c);
    }
}

void png::unfilter_line_paeth(std::span<uint8_t> line, std::span<uint8_t const> prev_line) const noexcept
{
    for (int i = 0; i != _bytes_per_line; ++i) {
        hilet j = i - _bytes_per_pixel;

        uint8_t const up = prev_line[i];
        uint8_t const left = j >= 0 ? line[j] : 0;
        uint8_t const left_up = j >= 0 ? prev_line[j] : 0;
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
    default: throw parse_error("Unknown line-filter type");
    }
}

void png::unfilter_lines(bstring &image_data) const
{
    hilet image_bytes = std::span(reinterpret_cast<uint8_t *>(image_data.data()), image_data.size());
    auto zero_line = std::vector<uint8_t>(_bytes_per_line, uint8_t{0});

    auto prev_line = std::span(zero_line.data(), zero_line.size());
    for (auto y = 0_uz; y != _height; ++y) {
        hilet line = image_bytes.subspan(y * _stride, _stride);
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
    hi_axiom(_bit_depth == 8 or _bit_depth == 16);
    hi_axiom(not _is_palletted);

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

    return u16x4{r, g, b, a};
}

void png::data_to_image_line(std::span<std::byte const> bytes, pixel_row<sfloat_rgba16> &line) const noexcept
{
    hilet alpha_mul = _bit_depth == 16 ? 1.0f / 65535.0f : 1.0f / 255.0f;
    for (int x = 0; x != _width; ++x) {
        hilet value = extract_pixel_from_line(bytes, x);

        hilet linear_RGB =
            f32x4{_transfer_function[value.x()], _transfer_function[value.y()], _transfer_function[value.z()], 1.0f};

        hilet linear_sRGB_color = _color_to_sRGB * linear_RGB;
        hilet alpha = static_cast<float>(value.w()) * alpha_mul;

        // pre-multiply the alpha for use in texture-maps.
        line[x] = linear_sRGB_color * alpha;
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
    hilet image_data_size = _stride * _height;

    auto image_data = decompress_IDATs(image_data_size);
    hi_parse_check(ssize(image_data) == image_data_size, "Uncompressed image data has incorrect size.");

    unfilter_lines(image_data);

    data_to_image(image_data, image);
}

pixel_map<sfloat_rgba16> png::load(std::filesystem::path const &path)
{
    hilet png_data = png(file_view{path});
    auto image = pixel_map<sfloat_rgba16>{png_data.width(), png_data.height()};
    png_data.decode_image(image);
    return image;
}

} // namespace hi::inline v1
