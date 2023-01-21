// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../file/file_view.hpp"
#include "../utility/module.hpp"
#include "../image/module.hpp"
#include "../geometry/module.hpp"
#include "../byte_string.hpp"
#include "../strings.hpp"
#include <span>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <filesystem>
#include <memory>

namespace hi::inline v1 {

class png {
public:
    [[nodiscard]] png(file_view view);

    [[nodiscard]] png(std::filesystem::path const& path) : png(file_view{path}) {}

    [[nodiscard]] std::size_t width() const noexcept
    {
        return _width;
    }

    [[nodiscard]] std::size_t height() const noexcept
    {
        return _height;
    }

    void decode_image(pixmap_span<sfloat_rgba16> image) const;

    [[nodiscard]] static pixmap<sfloat_rgba16> load(std::filesystem::path const& path);

private:
    /** Matrix to convert png color values to sRGB.
     * The default are sRGB color primaries and white-point.
     */
    matrix3 _color_to_sRGB = geo::identity();

    /** The gamma curve to convert a sample directly to linear float.
     */
    std::vector<float> _transfer_function;

    int _width = 0;
    int _height = 0;
    int _bit_depth = 0;
    int _color_type = 0;
    int _compression_method = 0;
    int _filter_method = 0;
    int _interlace_method = 0;

    bool _has_alpha;
    bool _is_palletted;
    bool _is_color;
    int _samples_per_pixel = 0;
    int _bits_per_pixel = 0;
    int _bytes_per_pixel = 0;
    int _bytes_per_line = 0;
    int _stride = 0;

    /** Spans of compressed data.
     */
    std::vector<std::span<std::byte const>> _idat_chunk_data;

    /** Take ownership of the view.
     */
    file_view _view;

    void read_header(std::span<std::byte const> bytes, std::size_t& offset);
    void read_chunks(std::span<std::byte const> bytes, std::size_t& offset);
    void read_IHDR(std::span<std::byte const> bytes);
    void read_cHRM(std::span<std::byte const> bytes);
    void read_gAMA(std::span<std::byte const> bytes);
    void read_iBIT(std::span<std::byte const> bytes);
    void read_iCCP(std::span<std::byte const> bytes);
    void read_sRGB(std::span<std::byte const> bytes);
    void generate_sRGB_transfer_function() noexcept;
    void generate_Rec2100_transfer_function() noexcept;
    void generate_gamma_transfer_function(float gamma) noexcept;
    [[nodiscard]] bstring decompress_IDATs(std::size_t image_data_size) const;
    void unfilter_lines(bstring& image_data) const;
    void unfilter_line(std::span<uint8_t> line, std::span<uint8_t const> prev_line) const;
    void unfilter_line_sub(std::span<uint8_t> line, std::span<uint8_t const> prev_line) const noexcept;
    void unfilter_line_up(std::span<uint8_t> line, std::span<uint8_t const> prev_line) const noexcept;
    void unfilter_line_average(std::span<uint8_t> line, std::span<uint8_t const> prev_line) const noexcept;
    void unfilter_line_paeth(std::span<uint8_t> line, std::span<uint8_t const> prev_line) const noexcept;
    void data_to_image(bstring bytes, pixmap_span<sfloat_rgba16> image) const noexcept;
    void data_to_image_line(std::span<std::byte const> bytes, std::span<sfloat_rgba16> row) const noexcept;
    u16x4 extract_pixel_from_line(std::span<std::byte const> bytes, int x) const noexcept;
};

} // namespace hi::inline v1
