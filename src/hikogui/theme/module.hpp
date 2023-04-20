
#pragma once

#include "theme_mode.hpp"
#include "theme_file.hpp"
#include "theme_book.hpp"
#include "theme_state.hpp"
#include "style_sheet_parser.hpp"
#include "style_sheet.hpp"

namespace hi {
inline namespace v1 {

/** @defgroup theme The theme system.
@ingroup GUI

Theme value names
-----------------

```
 ( ( name '/' )? widget '/' ) ( component '/' )? value ( ':' color-state )*
```

### Color states


  State            | Description
 :---------------- |:----------------------------------------------------------------------
  :disabled        | The widget is disabled.
  :enabled         | The widget is enabled.
  :hover           | The mouse hovers over the widget.
  :clicked         | The widget is being clicked by the mouse.
    (none)         | The color is set for each of these states.

  State            | Description
 :---------------- |:----------------------------------------------------------------------
  :no-focus        | The widget has no keyboard focus
  :focus           | The widget has keyboard focus
    (none)         | The color is set for both focus and non-focus.

  State            | Description
 :---------------- |:----------------------------------------------------------------------
  :off             | The widget's value is "off"
  :on              | The widget's value is "on"
    (none)         |




### Values

  Value            | Type       | Description
 :---------------- |:---------- |:--------------------------------------------------------------
  margin           | colors     | The color.
  spacing          | colors     | The color.
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

