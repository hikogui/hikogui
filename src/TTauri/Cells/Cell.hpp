// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/GUI/Window_forward.hpp"
#include "TTauri/Foundation/aarect.hpp"

namespace tt {
class DrawContext;

class Cell {
public:
    Cell() = default;
    virtual ~Cell() = default;
    Cell(Cell const &) noexcept = delete;
    Cell(Cell &&) noexcept = delete;
    Cell &operator=(Cell const &) noexcept = delete;
    Cell &operator=(Cell &&) noexcept = delete;

    /** Prepare the cell for drawing its contents.
     */
    virtual void prepareForDrawing(Window &device) noexcept = 0;

    /** Draw the image.
    *
    * @param drawContext The current draw context.
    * @param rectangle The position and size of the image.
    * @param alignment The alignment within the rectangle.
    * @return true when a redraw is needed.
    */
    [[nodiscard]] virtual bool draw(DrawContext const &drawContext, aarect rectangle, Alignment alignment) noexcept = 0;
};

}