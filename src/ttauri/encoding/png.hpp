

#pragma once

#include "../required.hpp"
#include "../PixelMap.hpp"
#include "../R16G16B16A16SFloat.hpp"
#include "../mat.hpp"
#include "../vec.hpp"
#include "../ivec.hpp"
#include "../URL.hpp"
#include "../ResourceView.hpp"
#include "../byte_string.hpp"
#include <nonstd/span>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace tt {

class png {
    /** Matrix to convert png color values to sRGB.
     * The default are sRGB color primaries and white-point.
     */
    mat color_to_sRGB = mat::I();

    /** The gamma curve to convert a sample directly to linear float.
     */
    std::vector<float> transfer_function;

    int width = 0;
    int height = 0;
    int bit_depth = 0;
    int color_type = 0;
    int compression_method = 0;
    int filter_method = 0;
    int interlace_method = 0;

    bool has_alpha;
    bool is_palletted;
    bool is_color;
    int samples_per_pixel = 0;
    int bits_per_pixel = 0;
    int bytes_per_pixel = 0;
    int bytes_per_line = 0;
    int stride = 0;

    /** Spans of compressed data.
     */
    std::vector<nonstd::span<std::byte const>> idat_chunk_data;

    /** Take ownership of the view.
     */
    std::unique_ptr<ResourceView> view;
public:

    png(nonstd::span<std::byte const> bytes);

    png(std::unique_ptr<ResourceView> view);

    png(URL const &url) :
        png(url.loadView()) {}

    ivec extent() const noexcept {
        return ivec{width, height};
    }

    void decode_image(PixelMap<R16G16B16A16SFloat> &image) const;

    static PixelMap<R16G16B16A16SFloat> load(URL const &url);

private:
    void read_header(nonstd::span<std::byte const> bytes, ssize_t &offset);
    void read_chunks(nonstd::span<std::byte const> bytes, ssize_t &offset);
    void read_IHDR(nonstd::span<std::byte const> bytes);
    void read_cHRM(nonstd::span<std::byte const> bytes);
    void read_gAMA(nonstd::span<std::byte const> bytes);
    void read_iBIT(nonstd::span<std::byte const> bytes);
    void read_iCCP(nonstd::span<std::byte const> bytes);
    void read_sRGB(nonstd::span<std::byte const> bytes);
    void generate_sRGB_transfer_function() noexcept;
    void generate_Rec2100_transfer_function() noexcept;
    void generate_gamma_transfer_function(float gamma) noexcept;
    bstring decompress_IDATs(ssize_t image_data_size) const;
    void unfilter_lines(bstring &image_data) const;
    void unfilter_line(nonstd::span<uint8_t> line, nonstd::span<uint8_t const> prev_line) const;
    void unfilter_line_sub(nonstd::span<uint8_t> line, nonstd::span<uint8_t const> prev_line) const noexcept;
    void unfilter_line_up(nonstd::span<uint8_t> line, nonstd::span<uint8_t const> prev_line) const noexcept;
    void unfilter_line_average(nonstd::span<uint8_t> line, nonstd::span<uint8_t const> prev_line) const noexcept;
    void unfilter_line_paeth(nonstd::span<uint8_t> line, nonstd::span<uint8_t const> prev_line) const noexcept;
    void data_to_image(bstring bytes, PixelMap<R16G16B16A16SFloat> &image) const noexcept;
    void data_to_image_line(nonstd::span<std::byte const> bytes, PixelRow<R16G16B16A16SFloat> &row) const noexcept;
    ivec extract_pixel_from_line(nonstd::span<std::byte const> bytes, int x) const noexcept;

};

}
