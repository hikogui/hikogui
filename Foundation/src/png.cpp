
#include "TTauri/Foundation/png.hpp"
#include "TTauri/Foundation/FileView.hpp"
#include "TTauri/Foundation/endian.hpp"
#include "TTauri/Foundation/placement.hpp"

namespace TTauri {

struct PNG:wq

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
