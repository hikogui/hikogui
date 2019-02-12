//
//  BackingPipeline.cpp
//  TTauri
//
//  Created by Tjienta Vara on 2019-02-12.
//  Copyright © 2019 Pokitec. All rights reserved.
//

#include "BackingPipeline.hpp"
#include "TTauri/Toolkit/Application.hpp"

namespace TTauri {
namespace Toolkit {
namespace GUI {

using namespace TTauri::Toolkit;

BackingPipeline::BackingPipeline(Window *window) :
    Pipeline(window, app->getPathToResource("BackingPipeline.vert.spv"), app->getPathToResource("BackingPipeline.frag.spv"))
{
}

BackingPipeline::~BackingPipeline()
{
}


}}}
