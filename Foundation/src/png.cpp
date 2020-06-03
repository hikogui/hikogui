
#include "TTauri/Foundation/png.hpp"
#include "TTauri/Foundation/endian.hpp"
#include "TTauri/Foundation/placement.hpp"
#include "TTauri/Foundation/sRGB.hpp"
#include "TTauri/Foundation/Rec2100.hpp"

namespace TTauri {

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

void png::read_header(nonstd::span<std::byte const> &bytes, ssize_t &offset)
{
    let png_header = make_placement_ptr<PNGHeader>(bytes);

    let valid_signature =
        png_header->signature[0] == 137 &&
        png_header->signature[1] == 80 &&
        png_header->signature[2] == 78 &&
        png_header->signature[3] == 71 &&
        png_header->signature[4] == 13 &&
        png_header->signature[5] == 10 &&
        png_header->signature[6] == 26 &&
        png_header->signature[7] == 10;

    parse_assert2(valid_signature, "invalid PNG file signature");
}

void png::generate_sRGB_transfer_function() noexcept
{
    let value_range = bit_depth == 8 ? 256 : 65536;
    let value_range_f = numeric_cast<float>(value_range);
    for (int i = 0; i != value_range; ++i) {
        auto u = numeric_cast<float>(i) / value_range_f;
        transfer_function.push_back(sRGB_gamma_to_linear(u));
    }
}

void png::generate_Rec2100_transfer_function() noexcept
{
    // SDR brightness is 80 cd/m2. Rec2100/PQ brightness is 10,000 cd/m2.
    constexpr float hdr_multiplier = 10'000.0f / 80.0f;

    let value_range = bit_depth == 8 ? 256 : 65536;
    let value_range_f = numeric_cast<float>(value_range);
    for (int i = 0; i != value_range; ++i) {
        auto u = numeric_cast<float>(i) / value_range_f;
        transfer_function.push_back(Rec2100_gamma_to_linear(u) * hdr_multiplier);
    }
}

void png::generate_gamma_transfer_function(float gamma) noexcept
{
    let value_range = bit_depth == 8 ? 256 : 65536;
    let value_range_f = numeric_cast<float>(value_range);
    for (int i = 0; i != value_range; ++i) {
        auto u = numeric_cast<float>(i) / value_range_f;
        transfer_function.push_back(powf(u, gamma));
    }
}


void png::read_IHDR(nonstd::span<std::byte const> &bytes)
{
    let ihdr = make_placement_ptr<IHDR>(bytes);

    width = ihdr->width.value();
    height = ihdr->height.value();
    bit_depth = ihdr->bit_depth;
    color_type = ihdr->color_type;
    compression_method = ihdr->compression_method;
    filter_method = ihdr->filter_method;
    interlace_method = ihdr->interlace_method;

    parse_assert2(width <= 16384, "PNG width too large.");
    parse_assert2(height <= 16384, "PNG height too large.");
    parse_assert2(bit_depth == 8 || bit_depth == 16, "PNG only bit depth of 8 or 16 is implemented.");
    parse_assert2(color_type == 2 || color_type == 6, "PNG only RGB or RGBA is implemented.");
    parse_assert2(compression_method == 0, "Only deflate/inflate compression is allowed.");
    parse_assert2(filter_method == 0, "Only adaptive filtering is allowed.");
    parse_assert2(interlace_method == 0, "Only non interlaced PNG are implemented.");

    generate_sRGB_transfer_function();
}

void png::read_cHRM(nonstd::span<std::byte const> &bytes)
{
    let chrm = make_placement_ptr<cHRM>(bytes);

    let color_to_XYZ = mat::RGBtoXYZ(
        numeric_cast<float>(chrm->white_point_x.value()) / 100'000.0f,
        numeric_cast<float>(chrm->white_point_y.value()) / 100'000.0f,
        numeric_cast<float>(chrm->red_x.value()) / 100'000.0f,
        numeric_cast<float>(chrm->red_y.value()) / 100'000.0f,
        numeric_cast<float>(chrm->green_x.value()) / 100'000.0f,
        numeric_cast<float>(chrm->green_y.value()) / 100'000.0f,
        numeric_cast<float>(chrm->blue_x.value()) / 100'000.0f,
        numeric_cast<float>(chrm->blue_y.value()) / 100'000.0f
    );

    color_to_sRGB = XYZ_to_sRGB * color_to_XYZ;
}

void png::read_gAMA(nonstd::span<std::byte const> &bytes)
{
    let gama = make_placement_ptr<gAMA>(bytes);
    let gamma = numeric_cast<float>(gama->gamma.value()) / 100'000.0f;
    parse_assert2(gamma != 0.0f, "Gamma value can not be zero");
     
    generate_gamma_transfer_function(1.0f / gamma);
}

void png::read_sRGB(nonstd::span<std::byte const> &bytes)
{
    let srgb = make_placement_ptr<sRGB>(bytes);
    let rendering_intent = srgb->rendering_intent;
    parse_assert2(rendering_intent <= 3, "Invalid rendering intent");

    color_to_sRGB = mat::I();
    generate_sRGB_transfer_function();
}

static std::string read_string(nonstd::span<std::byte const> &bytes)
{
    std::string r;

    for (ssize_t i = 0; i != ssize(bytes); ++i) {
        auto c = static_cast<char>(bytes[i]);
        if (c == 0) {
            return r;
        } else {
            r += c;
        }
    }
    TTAURI_THROW(parse_error("string is not null terminated."));
}

void png::read_iCCP(nonstd::span<std::byte const> &bytes)
{
    auto profile_name = read_string(bytes);

    if (profile_name == "ITUR_2100_PQ_FULL") {
        // The official rule here is to ignore everything in the ICC profile and
        // create the conversion matrix and transfer function from scratch.

        color_to_sRGB = XYZ_to_sRGB * Rec2100_to_XYZ;
        generate_Rec2100_transfer_function();
        return;
    }
}

void png::read_chunks(nonstd::span<std::byte const> &bytes, ssize_t &offset)
{
    auto IHDR_bytes = nonstd::span<std::byte const>{};
    auto cHRM_bytes = nonstd::span<std::byte const>{};
    auto gAMA_bytes = nonstd::span<std::byte const>{};
    auto iCCP_bytes = nonstd::span<std::byte const>{};
    auto sRGB_bytes = nonstd::span<std::byte const>{};
    bool has_IEND = false;

    while (!has_IEND) {
        let header = make_placement_ptr<ChunkHeader>(bytes, offset);
        let length = numeric_cast<ssize_t>(header->length.value());
        parse_assert2(length < 0x8000'0000, "Chunk length must be smaller than 2GB");
        parse_assert2(offset + length <= ssize(bytes), "Chuck extents beyond file.");

        switch (fourcc(header->type)) {
        case fourcc("IDAT"):
            compressed_data.push_back(bytes.subspan(offset, length));
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

        [[maybe_unused]] let crc = make_placement_ptr<big_uint32_buf_t>(bytes, offset);
    }

    parse_assert2(!IHDR_bytes.empty(), "Missing IHDR chunk.");
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

png::png(nonstd::span<std::byte const> bytes)
{
    ssize_t offset = 0;

    read_header(bytes, offset);
    read_chunks(bytes, offset);
}


}
