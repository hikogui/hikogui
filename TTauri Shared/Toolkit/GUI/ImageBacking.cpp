//
//  ImageBacking.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "ImageBacking.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

ImageBacking::ImageBacking(std::weak_ptr<Window> window, float2 size, boost::filesystem::path path) :
    Backing(window, size), path(path)
{
}

ImageBacking::~ImageBacking()
{
}

}}}
