

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include "TTauri/Foundation/mat.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/ivec.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/FileView.hpp"
#include <nonstd/span>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace TTauri {

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

    /** Spans of compressed data.
     */
    std::vector<nonstd::span<std::byte const>> compressed_data;

public:

    png(nonstd::span<std::byte const> bytes);

    png(URL const &url) :
        png(FileView(url)) {}

    ivec extent() const noexcept {
        return ivec{width, height};
    }

private:
    void read_header(nonstd::span<std::byte const> &bytes, ssize_t &offset);
    void read_chunks(nonstd::span<std::byte const> &bytes, ssize_t &offset);
    void read_IHDR(nonstd::span<std::byte const> &bytes);
    void read_cHRM(nonstd::span<std::byte const> &bytes);
    void read_gAMA(nonstd::span<std::byte const> &bytes);
    void read_iBIT(nonstd::span<std::byte const> &bytes);
    void read_iCCP(nonstd::span<std::byte const> &bytes);
    void read_sRGB(nonstd::span<std::byte const> &bytes);
    void generate_sRGB_transfer_function() noexcept;
    void generate_Rec2100_transfer_function() noexcept;
    void generate_gamma_transfer_function(float gamma) noexcept;

};

}
