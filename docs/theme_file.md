Theme file format
=================

HikoGUI theme files are based upon CSS (Cascading Style Sheets).


Syntax
------

```
Line comments start with "//" and block comments are surrounded by "/*" and "*/".
Both comments and white-space are ignored in the grammar below.

stylesheet := ( at_rule | ruleset )*

at_rule := theme-name | theme-mode | color-def | variable-def | macro-def
theme-name := '@' "name" ':' string ';'

theme-mode := '@' "mode" ':' theme-mode-name ';'
theme-mode-name := "dark" | "light";

color-def := '@' "color" color-name ':' color-value ';'
color-name := basic-color-name | gray-color-name
basic-color-name := "black" | "silver" | "gray" | "white" | "maroon" | "red"
                  | "purple" | "fuchsia" | "green" | "line" | "olive" | "yellow"
                  | "navy" | "blue" | "teal" | "aqua" | "indigo" | "orange"
                  | "pink"
gray-color-name := "background" | "gray1" | "gray2" | "gray3" | "gray4"
                 | "gray5" | "gray6" | "gray7" | "gray8" | "gray9"
                 | "foreground"

variable-def := '@' "let" variable-name ':' value ';'
variable-name := id

macro-def := '@' "macro" macro-name '{' declaration* '}'
macro-name := id

ruleset := selector '{' declaration* '}'

selector := pattern ( ',' pattern )* state*
pattern := element ( '>'? element )*
element := id | '*'

state := mouse-state | keyboard-state | widget-value | text-language | text-phrasing
mouse-state := ':' "disabled" | ':' "enabled" | ':' "hover" | ':' "active"
keyboard-state := ':' "no-focus" | ':' "focus"
widget-value := ':' "on" | ':' "off"
text-language := ':' "lang" '(' id ')'
text-phrasing := ':' "phrasing" '(' id ')'

declaration := property-declaration | macro-invocation
property-declaration := "font-weight" ':' font-weight ( '!' "important" )? ';'
                      | "font-style" ':' font-style ( '!' "important" )? ';'
                      | property ':' value ( '!' "important" )? ';'
property := id
macro-invocation := '@' macro-name ';'

value := length | lengths | color | variable-value | string

variable-value := '@' variable-name

color := color-name | color-value
color-value := hex-color | rgb-color
hex-color := '#' [0-9a-fA-F]{6,8}
color-component := number '%' | float | int
alpha-component := number '%' | float
rgb-color := "rgb" '(' color-component ','? color-component ','? color-component ( [,/]? alpha-component )? ')'

length := px-length | pt-length | cm-length | in-length | em-length
em-length := number "em"
px-length := number "px"
cm-length := number "cm"
in-length := number "in"
pt-length := number "pt"?

lengths := length length length*

font-style := "normal" | "italic" | "oblique"

font-weight := font-weight-number | font-weight-name
font-weight-number := 100 | 200 | 300 | 400 | 500 | 600 | 700 | 800 | 900 | 950
font-weight-name := "thin" | "extra-light" | "light" | "regular" | "normal"
                  | "medium" | "semi-bold" | "bold" | "extra-bold" | "black"
                  | "extra-black"
                 
number := float | int
```

Header
------

```css
@name: "Jurasic";
@mode: dark;
```

The following modes are available:
 - light,
 - dark.

Selectors
---------

### States

  State            | Description
 :---------------- |:--------------------------------------------------------------------------------------
  `:disabled`      | The widget is disabled, often shows somewhat transparent
  `:enabled`       | The widget is enabled.
  `:hover`         | The mouse hovers over the widget.
  `:active`        | The widget is clicked by the mouse or another indication that it is being activated
  `:no-focus`      | The widget does not have keyboard-focus
  `:focus`         | The widget has keyboard-focus
  `:on`            | The widget's value is "on"
  `:off`           | The widget's value is "off"
  `:layer()`       | The widget's layer modulo 4.
  `:lang()`        | The text is in/from a specific language, script or region.
  `:phrasing()`    | The text is from a set of phrasings.

#### :lang()

This selector applies to text of a specific language.
The argument of the `:lang()` selector is a IETF language tag. A standard
IETF language tag may have an optional _script_ and _region_ parts, as an
extension the tag here may have a `*` to indicate "any language".

#### :phrasing()

This selector applies to text of a specific phrasing. The argument of the
`:phrasing()` selector is one or more characters each specifying a phrasing.

  Character  | Enum          | Description                       | Common visual presentation
 ----------- |:------------- |:--------------------------------- |:------------------
  r          | regular;      | Regular text.                     | regular
  e          | emphasis;     | Said with importance.             | italic
  s          | strong;       | Said with raised voice.           | bold
  c          | code;         | A piece of computer code.         | monospaced
  a          | abbreviation; | A abbreviation                    | thin capitals, double underlined
  q          | quote;        | quoted from someone               | cursive
  k          | keyboard;     | A letter on the keyboard          | rounded box, inverted colors
  h          | highlight;    | A text that is highlighted        | yellow background
  m          | math          | A mathematical expression         | math-font
  x          | example       | Console commands                  | monospaced, low-pixel-font
  u          | unarticulated |                                   | underlined
  t          | title         | A heading                         | bold, larger
  S          | success       | Result / state indicating success | green
  W          | warning       | Result / state indicating warning | yellow
  E          | error         | Result / state indicating error   | red

Properties
----------

   Name                        | Types              
 :---------------------------- |:-------------------
   width                       | length
   height                      | length
   margin                      | length, (v h) (t h b) (t r b l)
   margin-left                 | length             
   margin-right                | length             
   margin-top                  | length             
   margin-bottom               | length
   spacing                     | length, (v h)
   spacing-vertical            | length             
   spacing-horizontal          | length             
   color                       | color
   caret-color                 | color, (primary, secondary)
   caret-color-primary         | color
   caret-color-secondary       | color
   selection-color             | color
   fill-color                  | color
   background-color            | color      
   border-width                | length             
   border-color                | color              
   border-radius               | length, (tl, tr, bl, br)
   border-top-left-radius      | length             
   border-top-right-radius     | length             
   border-bottom-left-radius   | length             
   border-bottom-right-radius  | length             
   font-family                 | string             
   font-type                   | font-type
   font-weight                 | font-weight
   font-size                   | length
   font-x-height               | [auto]
   font-cap-height             | [auto]
   font-line-height            | [auto]

Types
-----

### Length

#### points (pt, cm, in)

You define a length in points using the `pt` suffix or just a number without
a suffix. Lengths in points will scale based on the DPI of the display.

```css
* {
    width: 650pt;
    height: 300pt;
}
```

Length suffixed with `cm` or `in` are directly converted into `pt`.

#### pixels (px)

You define a length in pixels using the `px` suffix. When a length is defined
in pixels it will not scale based on the DPI of the display.

```css
* {
    width: 650pt;
    height: 300pt;
    border-width: 1px;
}
```

#### Em (em)

You define a length in Em using the `em` suffix. Lengths in Em will scale based
on the size of the selected font size (which may be defined in either point or pixels).

The size of a font specifies the line height, this includes the ascender,
descender and line-gap; this size equals `1 em`. The cap-height and x-height
are specific for each font, but the rough guideline for the cap-height is
about `0.7 em` and the x-height is about `0.48 em`.

```css
* {
    font-family: "New Times Roman";
    font-size: 12pt;
    height: 2 em;
}
```

### Color

#### Color name
A named color is defined using the `@color` rule and may be used anywhere
a color is used. The named colors are one from the following list:

 - black, silver, gray, white, maroon, red, purple, fuchsia, green, line,
 - olive, yellow, navy, blue, teal, aqua, indigo, orange, pink
 - gray-0, gray-10, gray-20, gray-30, gray-40, gray-50, gray-60, gray-70,
 - gray-80, gray-90, gray-100

The colors above are not completely fixed and should be themed to match the
style of the rest of the theme. For example these may be pastel colors or
with reduced saturation, or color shifted, or more balanced lightness.

The `gray-*` colors are defined as a percentage between the background (`gray-0`)
and foreground (`gray-100`) colors. This makes it possible to use the `gray-*`
names in the application independent if the theme is in light-mode or dark-mode. 

```css
@color black rgb(0.0, 0.0, 0.0);
@color white #ffffff;
@color red rgb(1.0, -0.2, -0.2);

* {
    color: white;
    border-color: red;
    background: black;
}
```

#### Hex color
The hex color component values range from 0 to 255 and are converted using the
sRGB transfer function to linear component values. A 4th value is used as
a linear alpha value where 0 is transparent and 255 is opaque

```css
* {
    color: #ffffff;
    border-color: #ff0000ff;
    background: #000000;
}
```

#### rgb()
A RGB color is defined using 3 (red, green, blue) or 4 (red, green, blue, alpha)
component values.

Colors are defined in the scRGB color space, this color space is 100 % compatible
with the sRGB color space, but can handle a larger color gamut and
high dynamic range.

Color component values come in the following variants:
 - floating point: The color component is treated as a linear value where
   `rgb(0.0 0.0 0.0)` is black, `rgb(1.0 1.0 1.0)` is D65 white at 80 cd/m2.
   values above 1.0 increases intensity linearly, negative values increases
   the color gamut.
 - integer: The color component value is between 0 and 255 and is converted
   using the sRGB transfer function (gamma) to linear. `rgb(0 0 0)` is black
   `rgb(255 255 255)` is D65 white at 80 cd/m2.
 - percentage: The color component value is between 0% and 100% and is converted
   using the sRGB transfer function (gamma) to linear. `rgb(0% 0% 0%)` is black
   `rgb(100% 100% 100%)` is D65 white at 80 cd/m2.

Alpha value come in the following variants:
 - floating point: The alpha component value is between 0.0 (transparent) and
   1.0 (opaque) and is treated as a linear opacity value.
 - percentage: The alpha component value is between 0 % (transparent) and
   100 % (opaque) and is treated as a linear opacity value.

```css
* {
    color: rgb(1.0, 1.0, 1.0);
    border-color: rgb(255, 0, 255 / 1.0);
    background: rgb(0% 0% 0% 100%);
}
```

### Font

In HikoGUI text may be in different languages and using different
(phrasings)[https://developer.mozilla.org/en-US/docs/Web/HTML/Content_categories#phrasing_content].

We can select different fonts, styles, weights, size and colors for each of those
languages and phrasings using selectors like in the example below.

```css
* {
    font-family: "Times New Roman";
    font-style: normal;
    font-weight: regular;
    font-size: 12pt;
    color: gray-100;
}

*:phrasing(eq) {
    font-style: italic;
}

*:phrasing(s) {
    font-weight: bold;
}

*:language(*-Hans) {
    font-family: "Microsoft YaHei UI";
    font-size: 11pt;
    color: gray-90;
}
```

#### font-family

#### font-style

The following font-styles are available;
 - normal,
 - oblique, italic


#### font-weight

  weight name     | weight number
 ---------------- |:------------
  thin            | 100
  extra-light     | 200
  light           | 300
  regular, normal | 400
  medium          | 500
  semi-bold       | 600
  bold            | 700
  extra-bold      | 800
  black           | 900
  extra-black     | 950



Variables
---------

```css
@let foo : 1pt;
@let bar : rgb(0.4, 0.5, 0.6);

* {
    border-width: @foo;
    border-color: @bar;
}
```

Macro
-----

```css
@macro foo {
    font-family: "New Times Roman";
    font-size: 12pt;
}

* {
    @foo;
    font-weight: bold;
}
```




