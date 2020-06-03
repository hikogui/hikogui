
#include "TTauri/Foundation/png.hpp"
#include "TTauri/Foundation/FileView.hpp"
#include "TTauri/Foundation/endian.hpp"
#include "TTauri/Foundation/placement.hpp"

namespace TTauri {

struct ChunkHeader {
    big_int32_buf_t length;    
    big_int32_buf_t type;    
};

struct ChunkTrailer {
    big_int32_buf_t crc32;    
};

struct IHDR {
    ChunkHeader header;
    big_int32_buf_t width;    
    big_int32_buf_t height;    
    uint8_t bit_depth;
    uint8_t color_type;
    uint8_t compression_method;
    uint8_t filter_method;
    uint8_t interlace_method;
    ChunkTrailer trailer;
};

struct gAMA {
    ChunkHeader header;
    big_int32_buf_t gamma;    
    ChunkTrailer trailer;
};

struct cHRM {
    ChunkHeader header;
    big_int32_buf_t white_point_x;    
    big_int32_buf_t white_point_y;    
    big_int32_buf_t red_x;    
    big_int32_buf_t red_y;    
    big_int32_buf_t green_x;    
    big_int32_buf_t green_y;    
    big_int32_buf_t blue_x;    
    big_int32_buf_t blue_y;    
    ChunkTrailer trailer;
};

struct sRGB {
    ChunkHeader header;
    uint8_t rendering_intent;
    ChunkTrailer trailer;
};

struct PNGReadState {
    placement_ptr<IHDR,std::byte const> ihdr;


};

std::pair<int,int> png_read_extent(nonstd::span<std::byte> bytes)
{

}


void png_decode(nonstd::span<std::byte> bytes, PixelMap<R16G16B16A16SFloat> &image)
{

}



PixelMap<R16G16B16A16SFloat> png_decode(nonstd::span<std::byte> bytes)
{

}

void png_decode(URL const &url, PixelMap<R16G16B16A16SFloat> &image)
{
    png_decode(FileView(url), image);
}

PixelMap<R16G16B16A16SFloat> png_decode(URL const &url)
{
    return png_decode(FileView(url));
}

}
