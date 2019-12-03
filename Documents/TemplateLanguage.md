# TTauri Template Language
This template language was designed for code generation for different programming languages
including HTML. The language was inspired by makotemplate.org, but with
a simpler data model making it more easy to do more complicated stuff.

## Data model
All assignments at the top level of a file will be added to the global scope.
All assignments inside an included file are also made to the global scope.

Inside functions all assignments are done in the local scope of the function.

Loops do not introduce a new scope, all assignments within a loop will therefor be in either
the global or local scope depending where the loop is defined. Some special read-only names
prefixed with one or more '$' characters are available inside the loop.

Functions and blocks exist in a special global scope which only contains functions.
`%function` or `%block` definitions which appear later in a file will override a previous
definition. By calling `super()` from within a `%function` or `%block` you can call its previous definition.

## Expressions
The `TTauri::datum` class is used for holding and operating on all the data while executing
an expression of the TTauri template language. A `TTauri::datum` class can hold the following types:
 * Undefined
 * None
 * Boolean
 * Integer
 * Decimal
 * Float
 * String
 * URL
 * Vector
 * Map

### Subexpression
Expressions can be enclosed inside parenthesis '(' ')' to force precedence on the expression inside.

### Operators

 | Precedence | Operator          | Description                          |
 | ----------:|:----------------- |:------------------------------------ |
 | 2          | expr '[' expr ']' | Subscripting                         |
 | 3          | '+' expr          | Unary plus                           |
 | 3          | '-' expr          | Unary minus                          |
 | 3          | '~' expr          | Bitwise not                          |
 | 3          | not expr          | Boolean not                          |
 | 4          | expr '**' expr    | Power                                |
 | 5          | expr '*' expr     | Multiply                             |
 | 5          | expr '/' expr     | Divide                               |
 | 5          | expr '%' expr     | Remainder                            |
 | 6          | expr '+' expr     | Addition / append                    |
 | 6          | expr '-' expr     | Subtraction                          |
 | 7          | expr '<<' expr    | Bitwise shift left                   |
 | 7          | expr '>>' expr    | Bitwise shift right with sign extend |
 | 9          | expr '<' expr     | Less than                            |
 | 9          | expr '>' expr     | Greater than                         |
 | 9          | expr '<=' expr    | Less than or equal to                |
 | 9          | expr '>=' expr    | Greater than or equal to             |
 | 10         | expr '==' expr    | Equal to                             |
 | 10         | expr '!=' expr    | Not equal to                         |
 | 11         | expr '&' expr     | Bitwise and                          |
 | 12         | expr '^' expr     | Bitwise xor                          |
 | 13         | expr '\|' expr    | Bitwise or                           |
 | 13         | expr '\|' name    | String filter                        |
 | 14         | expr 'and' expr   | Logical and                          |
 | 15         | expr 'or' expr    | Logical or                           |
 | 17         | expr ',' expr     | Comma separator on function calls    |

### Function call
Call a function with zero or more arguments.

A function is a name in special global scope with functions.
Functions are build-in, passed by the developer when evaluating an expression,
or added in the template by the user.

Syntax: name '(' ( expression ( ',' expression )* ','? )? ')'

### Null
Used by the user to denote a *nothing* value.

Syntax: `null`

### Undefined
Used by the system to denote a *nothing* value. This will be used
temporarily when indexing a vector or map on a non existing key which may then be replaced
with a new value.

Syntax: `undefined`

### Boolean
Syntax: `true` or `false`

Explicit conversion: `bool()`

Operations available that work on a boolean (all datum will be implicitly converted to a boolean):
 - `not boolean -> boolean`
 - `boolean and boolean -> boolean`
 - `boolean or boolean -> boolean`
 - `boolean == boolean -> boolean`
 - `boolean != boolean -> boolean`
 - `boolean < boolean -> boolean`
 - `boolean > boolean -> boolean`
 - `boolean <= boolean -> boolean`
 - `boolean >= boolean -> boolean`

### Integer
A 64 bit signed integer in 2's compliment format.

Syntax: `[+-]?(0[bBoOdDxX])?[0-9a-fA-F']+`

Explicit conversion: `int()`

Operations available that work on a integer:
 - `+ integer -> integer`
 - `- integer -> integer`
 - `integer + integer -> integer`
 - `integer - integer -> integer`
 - `integer * integer -> integer`
 - `integer / integer -> integer`
 - `integer % integer -> integer`
 - `integer ** integer -> integer`
 - `integer | integer -> integer`
 - `integer & integer -> integer`
 - `integer ^ integer -> integer`
 - `~ integer -> integer`
 - `integer >> integer -> integer`
 - `integer << integer -> integer`
 - `integer == integer -> boolean`
 - `integer != integer -> boolean`
 - `integer < integer -> boolean`
 - `integer > integer -> boolean`
 - `integer <= integer -> boolean`
 - `integer >= integer -> boolean`


### Decimal
A decimal floating point format in the form `mantissa + 10**exponent`.

No literal available.

Explicit conversion: `decimal()`

Operations available that work on a decimal float (integer are implicitly converted to decimal):
 - `+ decimal -> decimal`
 - `- decimal -> decimal`
 - `decimal + decimal -> decimal`
 - `decimal - decimal -> decimal`
 - `decimal * decimal -> decimal`
 - `decimal / decimal -> decimal`
 - `decimal % decimal -> decimal`
 - `decimal ** decimal -> decimal`
 - `decimal == decimal -> boolean`
 - `decimal != decimal -> boolean`
 - `decimal < decimal -> boolean`
 - `decimal > decimal -> boolean`
 - `decimal <= decimal -> boolean`
 - `decimal >= decimal -> boolean`
 - `round(decimal)`
 - `floor(decimal)`
 - `ceil(decimal)`


### Float
A binary floating point format in the form `mantissa + 2**exponent`.

Syntax: `[0-9]+.[0-9]*([eE][-]?[0-9]+)?` or `[0-9]*.[0-9]+([eE][-]?[0-9]+)?`

Explicit conversion: `float()`

Operations available that work on a binary float (integer and decimals are implicitly converted to float):
 - `+ float -> float`
 - `- float -> float`
 - `float + float -> float`
 - `float - float -> float`
 - `float * float -> float`
 - `float / float -> float`
 - `float % float -> float`
 - `float ** float -> float`
 - `float == float -> boolean`
 - `float != float -> boolean`
 - `float < float -> boolean`
 - `float > float -> boolean`
 - `float <= float -> boolean`
 - `float >= float -> boolean`
 - `round(float)`
 - `floor(float)`
 - `ceil(float)`

### String
Syntax: `"([^"]|\\")*"`

The literal string may include escape sequences:
 - `\"` A literal double quote
 - `\n` A literal line feed
 - `\r` A literal carriage return
 - `\t` A literal tab
 - `\f` A literal form feed

Explicit conversion can be done using the `string()` function.

Operations available that work on a string:
 - `string + string -> string`
 - `string == string -> boolean`
 - `string != string -> boolean`
 - `string < string -> boolean`
 - `string > string -> boolean`
 - `string <= string -> boolean`
 - `string >= string -> boolean`
 - `string[integer] -> string`
 - `substr(string text, integer start, integer length) -> string`
 - `size(string text) -> integer`
 - `string | name -> string` Pass the string through the named-filter.
    All datums are implicitly converted to a string.

Available filters:
 - `url` For text that needs to be encoded inside a URL.
 - `xml` For text that needs to be encoded inside a XML or HTML document.

### URL
No literal available.

Explicit conversion: `url()`

Operations available that work on a string:
 - `url + url -> url`
 - `url + string -> url`
 - `url == url -> boolean`
 - `url != url -> boolean`
 - `url < url -> boolean`
 - `url > url -> boolean`
 - `url <= url -> boolean`
 - `url >= url -> boolean`

### Vector
A list of `TTauri::datum` objects.

Syntax: '[' ( expression ( ',' expression )* ','? )? ']'

No explicit conversion available.

Operations available that work on a vector:
 - `vector + vector -> vector`
 - `vector == vector -> boolean`
 - `vector != vector -> boolean`
 - `vector < vector -> boolean`
 - `vector > vector -> boolean`
 - `vector <= vector -> boolean`
 - `vector >= vector -> boolean`
 - `vector[integer] -> datum`
 - `size(vector) -> integer`
 - `append(vector, datum value)`


### Map
A unordered map of key / value pairs; both `TTauri::datum` objects.

Syntax: '{' ( expression ':' expression ( ',' expression ':' expression )* ','? )? '}'

Operations available that work on a map:
 - `map + map -> map`
 - `map == map -> boolean`
 - `map != map -> boolean`
 - `map < map -> boolean`
 - `map > map -> boolean`
 - `map <= map -> boolean`
 - `map >= map -> boolean`
 - `map[datum] -> datum`
 - `size(map) -> integer`
 - `contains(map, datum) -> boolean`
 - `keys(map) -> vector`
 - `values(map) -> vector`


## Statements and placeholders

### Escape sequences
The backslash is used to escape:
 - End of line, mostly used for formatting inside loops.
 - The `$` dollar character used in placeholders.
 - The `#` hash character used in template statements.
 - The `\` backslash itself.

Example:
```
This placeholder is suppressed: \${12 + 24}
This backslash is suppressed: \\${12 + 24}
This line\
feed is suppressed.
```

Result:
```
This placeholder is suppressed: ${12 + 24}
This backslash is suppressed: \36
This linefeed is suppressed.
```

### Placeholder
The placeholder is an expression that is evaluated and explicitly converted to a string.
This string will then be inserted into the text.

Syntax: '${' expression '}'

Example:
```
${12 + 24}
```

Result:
```
36
```

### Assignment
Assignment of a value to a name. Later assignment overwrite the value that the name holds.

At the top-level assignments are done in global scope. Inside functions and blocks assignments
are done in the local scope, even if the name already exists in the global scope.
Loops do not introduce scopes.

Syntax: '#let' name (',' name)* = expression

Example
```
#let foo = 42
#let foo, bar = [foo + 2, 2]
${foo} ${bar}
```

Result:
```
44 2
```

### Including files
The `#include` statement is evaluated during parsing, by parsing the included file at the current position
recursively. An `#include` statement can only appear at the top-level of each file, i.e. it can not be used 
inside the body of a flow-control statement. 

Functions and blocks that are included by the `#include` statements are available in the global scope. See
the data model chapter.

When the filename argument is relative, the file is located relative to the current file.

There is no protection against including a file multiple times or recursively.

Syntax BNF: '#include' url-expression

It is recommended that the included files have the `.tti` (TTauri Include) extension,
and top-level template files have the `.ttt` (TTauri Template) extension.

### If statement
Conditional `#if` statement, with optional `#elif` statments and optional end `#else` statement.
The expression in the `#elif` statements are only evaluated if the result of the previous `#if` or `#elif`
expressions where `false`.

Syntax:
 - '#if' boolean-expression '\n'
 - '#elif' boolean-expression '\n'
 - '#else' '\n'
 - '#end' '\n'

Example:

```
#let foo = 5

#if foo == 2
Foo is two.
#elif foo == 3
Foo is three.
#elif foo == 4
Foo is four.
#else
Foo is ${foo}.
#end
```

Result:
```
Foo is 5.
```

### For loop
A for loop iterates over the result of an the expression. Each iteration-result is
assigned to the name in front of the `in` keyword, optionally the iteration-result is
unpacked into multiple names.

It is possible to loop over strings, vectors and maps. A string will yield a single character string
on each iterations. A vector will yield a value on each iteration. A map will yield a vector with a key
and value on each iteration, sorted by key.

When unpacking multiple names, the number of names must match the size of the vector that was yielded
on each iteration.

The `#else` part of the for loop is only executed when the result of the expression
has zero items.

Inside the loop extra variables are available for convenience:
 - `$i` integer index of the iteration
 - `$first` is true if this is the first iteration
 - `$last` is true if this is the last iteration
 - The extra variables created by an outer loop are prefixed with an extra `$`.

Syntax:
 - '#for' name ( ',' name )* 'in' expression '\n'
 - '#else' '\n'
 - '#end' '\n'

Example:
```
#for x in [1, 2, "hello"]
The value of x is ${x}.
#else
The list was empty.
#end
```

Result:
```
The value of x is 1.
The value of x is 2.
The value of x is hello.
```

### While loop
A while loop executes a block multiple times until the expression yields `false`.

Inside the loop extra variables are available for convenience:
 - `$i` integer index of the iteration
 - `$first` is true if this is the first iteration
 - The extra variables created by an outer loop are prefixed with an extra `$`.

Syntax:
 - '#while' boolean-expression '\n'
 - '#end' '\n'

Example:
```
#let i = 0
#while i < 3
Iteration ${i}.
#let i = i + 1
#end
```

Result:
```
Iteration 0.
Iteration 1.
Iteration 2.
```

### Do-while loop
A do-while loop executes a block at least once until the expression yields `false`.

Inside the loop extra variables are available for convenience:
 - `$i` integer index of the iteration
 - `$first` is true if this is the first iteration
 - The extra variables created by an outer loop are prefixed with an extra `$`

Syntax:
 - '#do' '\n'
 - '#while' boolean-expression '\n'

```
#let i = 0
#do
Iteration ${i}.
#let i = i + 1
#while i < 0
```

Result:
```
Iteration 0.
```

### Continue and Break
Stop executing of a block inside a loop, then:
 - *continue* with the next iteration of the loop or
 - *break* out of the loop

The argument to `#continue` and `#break` denotes the number of levels of outer-loops to break out of
or continue with. 1 denotes the current loop, 2 denotes the next outer-loop. Both the `#continue` and
`#break` statements have an implicit argument with the value of 1.

Syntax:
 - '#continue' ( integer-expression )? '\n'
 - '#break' ( integer-expression )? '\n'

Example:
```
#for x in ["foo", "bar", "baz"]
    #if x == "bar"
        #continue
The value of x is ${x}.
#end

#for y in ["foo", "bar", "baz"]
    #if y == "bar"
        #break
The value of y is ${y}.
#end
```

Result:
```
The value of x is foo.
The value of x is baz.
The value of y is foo.
```

### Function
Define a function that can be called in expressions.
A function with a return statement will simply return with its value.
A function without a return statement will return its textual-output.

It is not possible for a function to return both textual-output and a value.

Functions with the same name will replace the previously defined function.
The previously defined function is available as `super()` inside the function.
This functionality together with the `#include` statement can be used for
as a simple form of object-oriented-polymorphism.

Syntax:
 - '#function' name '(' ( name ( ',' name )* ','? )? ')' '\n'
 - '#end' '\n'

Example:
```
#function foo(x)
foo is ${x}.
#end

#function foo(x)
bar is ${super(x)}.
#end

${foo(42)}
```

Result:
```
bar is foo is 42.
.
```

### Return
Return data from a function

Syntax: '#return' expression '\n'

Example:
```
#function foo()
    #return 42
#end

${foo + 3}
```

Result:
```
45
```

### Named block
A block is like a function without arguments which is automatically evaluated
where it was first defined. A block can only return textual-output.

Like a function a block can be overridden by another block definition with the
same name. The previously defined block is available as `super()`. This functionality
together with the `#include` statement can be used for as a simple form of
object-oriented-polymorphism.

Syntax:
 - '#block' name '\n'
 - '#end'

Example:
```
1
#block foo
foo
#end
2
#block foo
bar
#end
3
```

Result:
```
1
bar
2
3
```
