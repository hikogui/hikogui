//
//  ImageView.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-04.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "ImageView.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

ImageView::ImageView(std::weak_ptr<View> view, const boost::filesystem::path &path) :
    View(view), path(path)
{
}

ImageView::~ImageView()
{
}

}}}
