Theme file format
=================



```

stylesheet := ( at_rule | ruleset | comment )*

at_rule := theme-name | theme-mode
theme-name := '@' "name" ':' string ';'
theme-mode := '@' "mode" ':' string ';'

ruleset := selector '{' ( declaration | comment )* '}'

selector := pattern ( ',' pattern )*

pattern := element
         | pattern element
         | pattern '>' element
         | pattern ':' state

state := mouse-state | keyboard-state | value-state
mouse-state := "disabled" | "enabled" | "hover" | "active"
keyboard-state := "no-focus" | "focus"
value-state := "on" | "off"

element := id | '*'

declaration := property ':' value ';'

property := '-' ? ( id '-' )* id



comment := line_comment | block_comment
line_comment := '/' '/' [^\n]* '\n'
block_comment := '/' '*' .* '*' '/'



value := length | color

px-length := number "px"
pt-length := number "pt"
length := px-length | pt-length

hex_color := '#' [0-9a-fA-F]{6,8}
int_color := "rgb" '(' int ',' int ',' int ( ',' int )? ')'
float_color := "rgb" '(' float ',' float ',' float ( ',' float )? ')'
pct_color := "rgb" '(' int '%' ',' int  '%'',' int '%' ( ',' int '%' )? ')'
color := hex_color | int_color | float_color | pct_color
```


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











