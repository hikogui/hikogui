Theme file format
=================

HikoGUI theme files are based upon CSS (Cascading Style Sheets).
The sele


Syntax
------

```

stylesheet := ( at_rule | ruleset | comment )*

at_rule := theme-name | theme-mode
theme-name := '@' "name" ':' string ';'
theme-mode := '@' "mode" ':' string ';'

font-theme := '@' "font-theme" id '{' declaration* sub-font-theme* '}'
sub-font-theme := sub-font-filter ( ',' sub-font-filter )? '{' declaration* '}'
sub-font-filter := "lang" '(' id ')' | "phrasing" '(' id ')'

ruleset := selector '{' ( declaration | comment )* '}'

selector := pattern ( ',' pattern )* ( ':' state )*
pattern := element ( '>'? element )*
element := id | '*'

state := "disabled" | "enabled" | "hover" | "active" | "no-focus" | "focus" | "on" | "off"

declaration := id ':' value ';'

comment := line_comment | block_comment
line_comment := '/' '/' [^\n]* '\n'
block_comment := '/' '*' .* '*' '/'

value := length | length2 | length4 | color | color-layers

color := hex_color | int_color | float_color | pct_color
color-layers := "color-layers" '(' color ( ',' color )* ')'

length := px-length | pt-length
length2 := length length
length4 := length length length length

px-length := number "px"
pt-length := number "pt"?

hex_color := '#' [0-9a-fA-F]{6,8}
int_color := "rgb" '(' int ',' int ',' int ( ',' int )? ')'
float_color := "rgb" '(' float ',' float ',' float ( ',' float )? ')'
pct_color := "rgb" '(' int '%' ',' int  '%'',' int '%' ( ',' int '%' )? ')'


```

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



Properties
----------


   Name                     | Types               |
 :----------------          |:------------------- |:--------------
   margin                   | Length, 4 x Length  |
   margin-left              | Length              |
   margin-right             | Length              |
   margin-top               | Length              |
   margin-bottom            | Length              |
   spacing                  | Length, 2 x Length  |
   spacing-vertical         | Length              |
   spacing-horizontal       | Length              |
   color                    | Color               |
   background-color         | Color               |
   border-width             | Length              |
   border-color             | Color               |
   border-radius            | Length, 4 x Length  |
   font-theme               | Font-theme id       |

Types
-----

### Length

```
```


### Color

```

```











