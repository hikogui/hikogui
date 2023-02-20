
/** @file theme/module.hpp Defined the theme module.
 */

#pragma once

namespace hi {
inline namespace v1 {

/** @defgroup theme The theme system.

Theme value names
-----------------

 <id>.<widget-type>.<component>.<value>:<state>

### States

  State            | Description
 :---------------- |:----------------------------------------------------------------------
  :disabled        | When the widget is disabled. Grayed out. (The widget won't react)
  :inactive        | The window is inactive. (The widget won't react)
                   | Normal state.
  :hover           | Mouse is hovering.
  :focus           | Widget has keyboard focus.
  :pressed         | Widget is clicked.
  :active          | Widget is active.
  :active:hover    | Widget is active and mouse is hovering
  :active:pressed  | Widget is active and is clicked.


### Values

  Value            | Type       | Description
 :---------------- |:---------- |:--------------------------------------------------------------
  .color           | colors     | The color.
  .small           | float      | The size of a small control, like a checkbox
  .medium          | float      | The size of a medium sized control, like the height of a selection box.
  .large           | float      | The size of the width a selection box.
  .margin          | float      | margin.
  .radius          | float      | corner radius.
  .width           | float      | thickness of lines.
  .baseline        | float      | Height of the text.
  .style           | text_theme | The text theme.

### Component

  Component        | Description
 :---------------- |:----------------------------------------------------------------------
  .border          | The border around the widget, often the one that shows focus.
  .outline         | The outline of the widget.

 */



}}

