// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Widget.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace TTauri::GUI::Widgets {

class ButtonWidget : public Widget {
protected:
    bool _value = false;
    bool _enabled = true;
    bool _focus = false;
    bool _pressed = false;
    std::string _label = "<unknown>";

    Path drawing;
    Text::ShapedText labelShapedText;
public:

    ButtonWidget(std::string const label) noexcept;
    ~ButtonWidget() {}

    ButtonWidget(const ButtonWidget &) = delete;
    ButtonWidget &operator=(const ButtonWidget &) = delete;
    ButtonWidget(ButtonWidget&&) = delete;
    ButtonWidget &operator=(ButtonWidget &&) = delete;

    [[nodiscard]] bool value() const noexcept { return _value; };
    ButtonWidget &set_value(bool rhs) noexcept { _value = rhs; _modified = true; return *this; }

    [[nodiscard]] bool enabled() const noexcept { return _enabled; };
    ButtonWidget &set_enabled(bool rhs) noexcept { _enabled = rhs; _modified = true; return *this; }

    [[nodiscard]] bool focus() const noexcept { return _focus; };
    ButtonWidget &set_focus(bool rhs) noexcept { _focus = rhs; _modified = true; return *this; }

    [[nodiscard]] bool pressed() const noexcept { return _pressed; };
    ButtonWidget &set_pressed(bool rhs) noexcept { _pressed = rhs; _modified = true; return *this; }

    [[nodiscard]] std::string label() const noexcept { return _label; };
    ButtonWidget &set_label(std::string rhs) noexcept { _label = std::move(rhs); _modified = true; return *this; }


    void update(bool modified) noexcept override;
    void pipelineImagePlaceVertices(gsl::span<GUI::PipelineImage::Vertex>& vertices, ssize_t& offset) noexcept override;
    void pipelineSDFPlaceVertices(gsl::span<GUI::PipelineSDF::Vertex>& vertices, ssize_t& offset) noexcept override;

    void handleMouseEvent(GUI::MouseEvent event) noexcept override;


private:
    PipelineImage::Backing::ImagePixelMap drawImage(std::shared_ptr<GUI::PipelineImage::Image> image) noexcept;

    PipelineImage::Backing backingImage; 
};

}
