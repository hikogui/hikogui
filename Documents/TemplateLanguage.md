# TTauri Template Language
The TTauri Template Language is designed for generate text documents.


## Data Types
### Boolean
### Integer
### Binary Float
### Decimal Float
### String
### URL
### Vector
### Map

## Expressions

### Literals

```
1234'3434'34
1234'3434'34.2
"foobar"
true
false
none
[<literal>, ...]
{<literal> : <literal>, ...}
```

### Function call
```
<name>()
<name>(<expression>[,<expression>])
```

### Index operator
```
<expression>[<expression>]

<expression>.<name>
```

### Arithmatic Operators

 | Op | Left-hand-side type     | Right-hand-side type    | Result  |
 +----+-------------------------+-------------------------+---------+
 | +  | integer                 | integer                 | integer |
 | +  | dec-flt/integer         | dec-flt/integer         | dec-flt |
 | +  | bin-flt/dec-flt/integer | bin-flt/dec-flt/integer | bin-flt |
 | +  | string                  | string                  | string  |
 | +  | url/string              | url/string              | url     |
 | +  | vector                  | vector                  | vector  |
 | +  | map                     | map                     | map     |
 | -  | integer                 | integer                 | integer |
 | -  | dec-flt/integer         | dec-flt/integer         | dec-flt |
 | -  | bin-flt/dec-flt/integer | bin-flt/dec-flt/integer | bin-flt |
 | -  |                         | integer                 | integer |
 | -  |                         | dec-flt                 | dec-flt |
 | -  |                         | bin-flt                 | bin-flt |
 | *  | integer                 | integer                 | integer |
 | *  | dec-flt/integer         | dec-flt/integer         | dec-flt |
 | *  | bin-flt/dec-flt/integer | bin-flt/dec-flt/integer | bin-flt |
 | /  | integer                 | integer                 | integer |
 | /  | dec-flt/integer         | dec-flt/integer         | dec-flt |
 | /  | bin-flt/dec-flt/integer | bin-flt/dec-flt/integer | bin-flt |
 | %  | integer                 | integer                 | integer |
 | %  | dec-flt/integer         | dec-flt/integer         | dec-flt |
 | %  | bin-flt/dec-flt/integer | bin-flt/dec-flt/integer | bin-flt |
 | ** | integer                 | integer                 | integer |
 | ** | bin-flt/dec-flt/integer | bin-flt/dec-flt/integer | bin-flt |

## Binary operations

 | Op | Left-hand-side type     | Right-hand-side type    | Result  |
 +----+-------------------------+-------------------------+---------+
 | &  | integer                 | integer                 | integer |
 | |  | integer                 | integer                 | integer |
 | ^  | integer                 | integer                 | integer |
 | >> | integer                 | integer                 | integer |
 | << | integer                 | integer                 | integer |
 | ~  |                         | integer                 | integer |

## Logical operations

 | Op | Left-hand-side type     | Right-hand-side type    | Result  |
 +----+-------------------------+-------------------------+---------+
 | && | any                     | any                     | boolean |
 | || | any                     | any                     | boolean |
 | == | any                     | any                     | boolean |
 | != | any                     | any                     | boolean |
 | <  | any                     | any                     | boolean |
 | >  | any                     | any                     | boolean |
 | <= | any                     | any                     | boolean |
 | >= | any                     | any                     | boolean |
 | !  |                         | any                     | boolean |

## Template Language

### Placeholder
```
${<expression>}
```

### If statement

```
#if <expression>
<text>
#elif <expression>
<text>
#else
<text>
#end
```

### For statement

```
#for name in <expression>
<text>
#else
<text>
#end
```

### Repeat statement

```
#repeat <expression>
<text>
#end
```

### While statement
```
#while <expression>
<text>
#else
<text>
#end
```

### Do statement
```
#do
<text>
#while <expression>
```

### Assign statement
```
#var <name> = <expression>
```

### Break/Continue/Exit

```
#break
#continue
#exit
```

### Include
```
#include <expression>
```

