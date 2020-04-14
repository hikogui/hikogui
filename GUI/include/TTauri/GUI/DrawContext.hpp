// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/mat.hpp"
#include "TTauri/Foundation/rect.hpp"
#include "TTauri/Foundation/vspan.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/PipelineImage_Image.hpp"
#include "TTauri/GUI/PipelineFlat_DeviceShared.hpp"
#include "TTauri/GUI/PipelineBox_DeviceShared.hpp"
#include "TTauri/GUI/PipelineImage_DeviceShared.hpp"
#include "TTauri/GUI/PipelineSDF_DeviceShared.hpp"
#include "TTauri/GUI/PipelineFlat_Vertex.hpp"
#include "TTauri/GUI/PipelineBox_Vertex.hpp"
#include "TTauri/GUI/PipelineImage_Vertex.hpp"
#include "TTauri/GUI/PipelineSDF_Vertex.hpp"
#include "TTauri/Text/ShapedText.hpp"
#include <type_traits>

namespace TTauri::GUI {

class DrawContext {
    Window &window;
    vspan<PipelineFlat::Vertex> &flatVertices;
    vspan<PipelineBox::Vertex> &boxVertices;
    vspan<PipelineImage::Vertex> &imageVertices;
    vspan<PipelineSDF::Vertex> &sdfVertices;

public:
    vec color = vec::color(1.0, 1.0, 1.0, 1.0);
    vec fillColor = vec::color(0.0, 0.0, 0.0, 0.0);
    vec borderColor = vec::color(1.0, 1.0, 1.0, 1.0);
    float borderSize = 1.0;
    float shadowSize = 0.0;
    vec cornerShapes = vec{0.0, 0.0, 0.0, 0.0};
    rect clippingRectangle;
    mat transform = mat::I();

    DrawContext(
        Window &window,
        vspan<PipelineFlat::Vertex> &flatVertices,
        vspan<PipelineBox::Vertex> &boxVertices,
        vspan<PipelineImage::Vertex> &imageVertices,
        vspan<PipelineSDF::Vertex> &sdfVertices
    ) noexcept :
        window(window),
        flatVertices(flatVertices),
        boxVertices(boxVertices),
        imageVertices(imageVertices),
        sdfVertices(sdfVertices),
        clippingRectangle(vec{0.0, 0.0}, static_cast<vec>(window.currentWindowExtent))
    {
        flatVertices.clear();
        boxVertices.clear();
        imageVertices.clear();
        sdfVertices.clear();
    }

    DrawContext(DrawContext const &rhs) noexcept = default;
    DrawContext(DrawContext &&rhs) noexcept = default;
    DrawContext &operator=(DrawContext const &rhs) noexcept = default;
    DrawContext &operator=(DrawContext &&rhs) noexcept = default;
    ~DrawContext() = default;

    DrawContext &drawFilledQuad(vec p1, vec p2, vec p3, vec p4) noexcept {
        flatVertices.emplace_back(transform * p1, clippingRectangle, fillColor);
        flatVertices.emplace_back(transform * p2, clippingRectangle, fillColor);
        flatVertices.emplace_back(transform * p3, clippingRectangle, fillColor);
        flatVertices.emplace_back(transform * p4, clippingRectangle, fillColor);
        return *this;
    }

    DrawContext &drawFilledQuad(rect r) noexcept {
        return drawFilledQuad(r.corner<0>(), r.corner<1>(), r.corner<2>(), r.corner<3>());
    }

    DrawContext &drawBox(rect r) noexcept {
        let p1 = transform * r.p1();
        let p2 = transform * r.p2();
        r = rect::p1p2(p1, p2);

        PipelineBox::DeviceShared::placeVertices(
            boxVertices,
            p1.z(),
            r,
            fillColor,
            borderSize,
            borderColor,
            shadowSize,
            cornerShapes,
            clippingRectangle
        );
        return *this;
    }

    DrawContext &drawImage(PipelineImage::Image &image) noexcept {
        image.placeVertices(imageVertices, transform, clippingRectangle);
        return *this;
    }

    DrawContext &drawText(Text::ShapedText &text) noexcept {
        window.device->SDFPipeline->placeVertices(
            sdfVertices,
            text,
            transform,
            clippingRectangle
        );
        return *this;
    }

    template<typename M, std::enable_if_t<is_mat_v<M>, int> = 0>
    friend DrawContext operator*(M const &lhs, DrawContext rhs) noexcept {
        rhs.transform = lhs * rhs.transform;
        return rhs;
    }
};

}