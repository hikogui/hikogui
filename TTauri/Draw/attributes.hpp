// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace TTauri::Draw {

enum class VerticalAlignment {
    Top,
    Middle,
    Base,
    Bottom
};

enum class HorizontalAlignment {
    Left,
    Center,
    Right
};

enum class Alignment {
    TopLeft,
    TopCenter,
    TopRight,
    MiddleLeft,
    MiddleCenter,
    MiddleRight,
    BaseLeft,
    BaseCenter,
    BaseRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

enum class LineJoinStyle {
    Bevel,
    Miter,
    Rounded
};

enum class SubpixelOrientation {
    RedLeft,
    RedRight,
    Unknown
};


}