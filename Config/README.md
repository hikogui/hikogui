# TTauri Config

This configuration file format is inspired by many types of configuration file formats
such as JSON and INI files. TTauri configuration files have access to an expression language
to calculate values to be assigned to configuration items.

## Types

### Null

```
a = null;
```

### Boolean

```
a = true;
b = false;
```

### Integer
The radix of the integer can be giving by using one of the following prefixes:
 * 0b - Binary
 * 0o - Octal
 * 0d - Decimal
 * 0x - Hexadecimal

Numbers without a prefix are decimal.

Negative numbers start with the minus `-` character before the radix prefix.
Numbers may contain `_` or `'` characters to group digits.

```
a = 12;     // Assign 12.
b = 0x40;   // Assign 64.
c = -0x20;  // Assign -32.
```

### Float

```
a = 1.5;    // Assign 1.5
b = .5;     // Assign 0.5
c = -1.;    // Assign -1.0
```

### String

```
a = "hello";
b = "foo \"bar\"";
c = "foo\nbar";
```

### Path
There is no direct path literal, a path can be created by using the functions `path()` and `cwd()`.
Concatenating path objects with strings will result in a new path.

### Color
```
a = #123456;
b = #123456ff;
```

### Array
```
a = [];
b = [1, 2, 3];
c = [1, 2, 3,];
```

### Object
```
a = 5; // Assign 5 to the root object.
b = {
    foo: 1; // Asign 1 to foo in the object b.
    include("bar.txt") // Add the root object parsed from "bar.txt" into object b.
}
```
## Operators

 | operator           | description                                                               |
 |:------------------ |:------------------------------------------------------------------------- |
 | ~ int              | Invert all bits of the int                                                |
 | - int              | Negate the int                                                            |
 | - float            | Negate float                                                              |
 | . id               | Access member of root object                                              |
 | $ id               | Access variable                                                           |
 | not any            | Invert boolean value, almost every type can be represented as a bool      |
 | lvalue = any       | Asign an expression to the left side. May be used inside an expression itself. It is not allowed to assign Undefined values |
 | object . id        | Access member of object                                                   |
 | int * int          | Multiply integers                                                         |
 | float * float      | Multiply floats                                                           |
 | int / int          | Divide integers                                                           |
 | float / float      | Divide floats                                                             |
 | int % int          | Take modulo of integers                                                   |
 | float % float      | Take modulo of floats                                                     |
 | int + int          | Add integers                                                              |
 | float + float      | Add floats                                                                |
 | string + string    | Concatonate strings                                                       |
 | path + path        | Concatonate path and string into a path.                                  |
 | int - int          | Subtract integers                                                         |
 | float - float      | Subtract floats                                                           |
 | int << int         | Shift left integer                                                        |
 | int >> int         | Shift right integer                                                       |
 | float < float      | Compare floats using epsilon                                              |
 | float > float      | Compare floats using epsilon                                              |
 | float <= float     | Compare floats using epsilon                                              |
 | float >= float     | Compare floats using epsilon                                              |
 | float != float     | Compare floats using epsilon                                              |
 | float == float     | Compare floats using epsilon                                              |
 | string < string    | Compare string lexographically                                            |
 | string > string    | Compare string lexographically                                            |
 | string <= string   | Compare string lexographically                                            |
 | string >= string   | Compare string lexographically                                            |
 | string != string   | Compare string lexographically                                            |
 | string == string   | Compare string lexographically                                            |
 | boolean < boolean  | Compare booleans                                                          |
 | boolean > boolean  | Compare booleans                                                          |
 | boolean <= boolean | Compare booleans                                                          |
 | boolean >= boolean | Compare booleans                                                          |
 | boolean != boolean | Compare booleans                                                          |
 | boolean == boolean | Compare booleans                                                          |
 | int & int          | AND all bits together of two integers                                     |
 | int \| int         | OR all bits together of two integers                                      |
 | int ^ int          | XOR all bits together of two integers                                     |
 | boolean & boolean  | Boolean AND                                                               |
 | boolean \| boolean | Boolean OR                                                                |
 | boolean ^ boolean  | Boolean XOR                                                               |
 | any and any        | AND two boolean values, almost every type can be represented as a boolean |
 | any or any         | OR two boolean values, almost every type can be represented as a boolean  |
 | any xor any        | XOR two boolean values, almost every type can be represented as a boolean |
 | array [ int ]      | Index and array by an integer                                             |
 | any ( any* )       | Call a function on an object with any argument                            |

`int` and `string` are promoted to `double` and `path` to match operands on both side of the operator.

## Functions

### include(path|string)
Include a file relative to the configuration file into the current object.

### path(path|string)
Transform string to a path relative to the configuration file.
An absolute path as argument remains absolute.

### cwd(path|string)
Transform string to a path relative to the current working directory.
An absolute path as argument is not allowed.


## Lexicon
```
BIN             [_01]
OCT             [_0-7]
DEC             [_0-9]
HEX             [_0-9a-fA-F]

STR             [^"\n\r]
STRDQ           "\\\""
WS              [ \t\f\r\n]

%%

[=:]                        '='
[;,]                        ';'

[_a-zA-Z][_a-zA-Z0-9]*      T_IDENTIFIER;

-0[bB]{BIN}+                T_INTEGER;
-0[oO]{OCT}+                T_INTEGER;
-0[dD]{DEC}+                T_INTEGER;
-0[xX]{HEX}+                T_INTEGER;
0[bB]{BIN}+                 T_INTEGER;
0[oO]{OCT}+                 T_INTEGER;
0[dD]{DEC}+                 T_INTEGER;
0[xX]{HEX}+                 T_INTEGER;
-{DEC}+                     T_INTEGER;
{DEC}+                      T_INTEGER;
#{HEX}{6}                   T_COLOR;
#{HEX}{8}                   T_COLOR;

-?"."{DEC}+                 T_FLOAT;
-?{DEC}+"."{DEC}*           T_FLOAT;

\"({STR}|{STRDQ})*?\"       T_STRING;

"//".*?\n                   ; // Comment is ignored.
{WS}                        ; // Whitespace is ignored.

## Syntax
Ignores comment and white-space-char tokens.

```
array:
      '[' ']'
    | '[' expressions ']'
    | '[' expressions ';' ']'
    ;

object:
      '{' '}'
    | '{' expressions '}'
    | '{' expressions ';' '}'
    ;

expression_without_array:
      '(' expression ')'
    | object
    | T_INTEGER
    | T_FLOAT
    | T_COLOR
    | "true"
    | "false"
    | "null"
    | T_STRING
    | T_IDENTIFIER
    | '~' expression
    | '-' expression %prec UMINUS
    | '.' T_IDENTIFIER
    | '$' T_IDENTIFIER
    | "not" expression
    | expression '=' expression
    | expression '.' T_IDENTIFIER
    | expression '*' expression
    | expression '/' expression
    | expression '%' expression
    | expression '+' expression
    | expression '-' expression
    | expression "<<" expression
    | expression ">>" expression
    | expression '<' expression
    | expression '>' expression
    | expression "<=" expression
    | expression ">=" expression
    | expression "==" expression
    | expression "!=" expression
    | expression '&' expression
    | expression '^' expression
    | expression '|' expression
    | expression "and" expression
    | expression "xor" expression
    | expression "or" expression
    | expression '[' expression ']'
    | expression '(' expressions ')'
    ;

expression:
      expression_without_array
    | array
    ;

expressions:
      expression
    | expressions ';' expression
    ;

statement:
      expression_without_array ';'
    | array
    ;

statements:
      statement
    | statements statement
    ;

root:
      /* empty */
    | object
    | statements
    ;
```

## Functions

### color(r: float, g: float, b: float, a: float=1.0)
Return a color object. Given r, g, b values are sRGB including gamma.
r, g, b values outside of the range 0.0-1.0 are allowed and are clamped to
the color space of the display.

### color(string)
Return a color object. The string is a '#' character followed by 6 or 8 hex digits
each pair denoting r, g, b, a values in sRGB color space including gamma.
