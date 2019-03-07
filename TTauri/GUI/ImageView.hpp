//
//  ImageView.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include <memory>
#include <boost/filesystem.hpp>
#include "View.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

class ImageView : View {
public:
    const boost::filesystem::path path;

    ImageView(View *view, const boost::filesystem::path &path);
    ~ImageView();
};

}}}
