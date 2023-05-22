
#pragma once

#include "style_sheet.hpp"
#include "style_sheet_parser.hpp"
#include "theme_book.hpp"
#include "theme_length.hpp"
#include "theme_mode.hpp"
#include "theme_model.hpp"
#include "theme_state.hpp"

namespace hi {
inline namespace v1 {

/** @defgroup theme The theme system.
@ingroup GUI

Theme value names
-----------------

```
 ( ( name '/' )? widget '/' ) ( component '/' )? value ( ':' color-state )*
```

### Theme states


  State            | Description
 :---------------- |:----------------------------------------------------------------------
  :disabled        | The widget is disabled.
  :enabled         | The widget is enabled.
  :hover           | The mouse hovers over the widget.
  :active          | The widget is being clicked by the mouse.
    (none)         | The model is set for every widget-state.
                   |
  :no-focus        | The widget has no keyboard focus.
  :focus           | The widget has keyboard focus.
    (none)         | The model is used for both focus and non-focus.
                   |
  :off             | The widget's value is "off".
  :on              | The widget's value is "on".
    (none)         | The model is used for both values.
                   |
  :layer(0)        | For widgets that are on layer % 4 = 0.
  :layer(1)        | For widgets that are on layer % 4 = 1.
  :layer(2)        | For widgets that are on layer % 4 = 2.
  :layer(3)        | For widgets that are on layer % 4 = 3.
    (none)         | The model is used for all layers.




### Values

  Value            | Type       | Description
 :---------------- |:---------- |:--------------------------------------------------------------
  margin           | colors     | The color.
  fill/color       | colors     | The color.
  outline/color    | colors     | The color.
  outline/width    | colors     | The color.
  outline/radius   | colors     | The color.
  style            | float      | margin.
  text/style       | float      | margin.
  icon/color       | float      | margin.
  cap-height       | float      | margin.
  size             | float      | margin.
  width             | float      | margin.
  height             | float      | margin.
  rail/length             | float      | margin.
  rail/breath             | float      | margin.
  icon/size        | float      | margin.
  pip/color        | float      | margin.
  pip/radius        | float      | margin.

### Component

  Component        | Description
 :---------------- |:----------------------------------------------------------------------
  .border          | The border around the widget, often the one that shows focus.
  .outline         | The outline of the widget.

 */



}}

