Theme file format
=================



```
Line comments start with "//" and block comments are surrounded by "/*" and "*/".
Both comments and white-space are ignored in the grammar below.

stylesheet := ( at_rule | ruleset )*

at_rule := theme-name | theme-mode
theme-name := '@' "name" ':' string ';'
theme-mode := '@' "mode" ':' string ';'

ruleset := selector '{' declaration* '}'

selector := pattern ( ',' pattern )* ( ':' state )*
pattern := element ( '>'? element )*
element := id | '*'

state := "disabled" | "enabled" | "hover" | "active" | "no-focus" | "focus" | "on" | "off"

declaration := id ':' value ';'

value := length | lengths | color | color-layers

color := hex_color | int_color | float_color | pct_color
color-layers := "color-layers" '(' color ( ',' color )* ')'

length := px-length | pt-length
lengths := length length length*

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

Types
-----

### Length

```
```


### Color

```

```











