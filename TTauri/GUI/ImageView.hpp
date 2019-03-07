//
//  ImageView.hpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#pragma once

#include "View.hpp"
#include <boost/filesystem.hpp>
#include <memory>

namespace TTauri {
namespace GUI {

class ImageView : View {
public:
    const boost::filesystem::path path;

    ImageView(View *view, const boost::filesystem::path &path);
    ~ImageView();
};

}}
