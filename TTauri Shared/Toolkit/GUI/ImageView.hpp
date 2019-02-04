//
//  ImageView.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <memory>
#include "View.hpp"
#include "ImageBacking.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

class ImageView : View {
public:
    boost::filesystem::path path;

    ImageView(std::weak_ptr<View> view, boost::filesystem::path path);
    ~ImageView();
};

}}}
