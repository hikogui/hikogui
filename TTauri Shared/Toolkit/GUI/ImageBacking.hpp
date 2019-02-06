//
//  ImageBacking.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <boost/filesystem.hpp>
#include "Backing.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

class ImageBacking : Backing {
public:
    const boost::filesystem::path path;

    virtual size_t hash(void) const {
        size_t seed = 0;
        boost::hash_combine(seed, Backing::hash());
        boost::hash_combine(seed, hash_value(path));
        return seed;
    }

    virtual bool operator==(const Backing &other) const {
        if (!Backing::operator==(other)) {
            return false;
        }

        if (auto _other = dynamic_cast<const ImageBacking *>(&other)) {
            return path == _other->path;
        } else {
            return false;
        }
    }

    ImageBacking(Window *window, float2 size, const boost::filesystem::path &path);
    virtual ~ImageBacking();
};

}}}
