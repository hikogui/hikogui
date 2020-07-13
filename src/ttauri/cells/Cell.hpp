// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../GUI/Window_forward.hpp"
#include "../aarect.hpp"

namespace tt {
class DrawContext;

class Cell {
protected:
    /** Set to true when the data of the cell has been modified.
     */
    mutable bool modified;

public:
    Cell() : modified(true) {}
    virtual ~Cell() = default;
    Cell(Cell const &) noexcept = delete;
    Cell(Cell &&) noexcept = delete;
    Cell &operator=(Cell const &) noexcept = delete;
    Cell &operator=(Cell &&) noexcept = delete;

    /** Return the extent that this cell wants to be drawn as.
     */
    [[nodiscard]] virtual vec preferredExtent() const noexcept { return {}; }

    /** Get the height to draw this cell for the given width.
     */
    [[nodiscard]] virtual float heightForWidth(float width) const noexcept { return 0.0f; }

    /** Draw the cell.
    *
    * @param drawContext The current draw context.
    * @param rectangle The position and size of the image.
    * @param alignment The alignment within the rectangle.
    * @param middle The height of the middle of the line of text.
    * @param useContextColor True to use the colors in the context, False to use the colors in the cell itself.
    */
    virtual void draw(
        DrawContext const &drawContext,
        aarect rectangle,
        Alignment alignment,
        float middle=std::numeric_limits<float>::max(),
        bool useContextColor=false
    ) const noexcept = 0;
};

}
