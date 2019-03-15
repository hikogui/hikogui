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

class ImageView : public View {
public:
    const boost::filesystem::path path;

    ImageView(const boost::filesystem::path &path);
    ~ImageView();

    virtual size_t BackingPipelineRender(BackingPipeline::Vertex *vertices, size_t offset, size_t size);

};

}
