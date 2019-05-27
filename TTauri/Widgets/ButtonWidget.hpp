// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/all.hpp"
#include <memory>
#include <string>
#include <array>

namespace TTauri::Widgets {

class ButtonWidget : public GUI::Widget {
public:
    enum class State {
        ENABLED,
        DISABLED,
        ACTIVE,
        PRESSED,
        HOVER
    };
    static constexpr size_t StateCount = 5;

    std::array<std::shared_ptr<GUI::PipelineImage::Image>, StateCount> imagePerState;

    std::string label;
    State state = State::ENABLED;

    ButtonWidget(std::string const label);
    ~ButtonWidget() {}

    ButtonWidget(const ButtonWidget &) = delete;
    ButtonWidget &operator=(const ButtonWidget &) = delete;
    ButtonWidget(ButtonWidget&&) = delete;
    ButtonWidget &operator=(ButtonWidget &&) = delete;


    void pipelineImagePlaceVertices(gsl::span<GUI::PipelineImage::Vertex>& vertices, size_t& offset) override;

protected:
    void drawImage(GUI::PipelineImage::Image &image, State state);

};

}
