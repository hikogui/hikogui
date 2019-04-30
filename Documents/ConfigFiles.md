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

### Path

### Color

### Array


### Object






## Examples

```
foo = 12;

baz = {
    name: "Hello"
};

foo.bar: 42; // Error, foo is not an object.

baz.value: "World";

[test]
// Creates object test, with foo = 13 key-value.
foo: 13;

// Includes the object from otherfile.txt and merge with the test object.
// Calls the include method on the test object.
include("otherfile.txt");

// Select the root object
[]

// Add more to the root object and calculate foo + 5.
more: foo + 5;

intlist: [1, 2, 3, 4];

// Insert 100 in front of intlist.
intlist.add(0, 100);

// Append 5 after intlist.
intlist.add(5);

// Insert 6 before the last entry.
intlist.add(-1, 6);

// Remove the 4th entry.
intlist.remove(3);

// Remove the last entry.
intlist.remove();

// Remove the last entry.
intlist.remove(-1);

// Delete the intlist key from the root object.
// Calls the delete object from the root object.
delete("intlist");
```


## Lexicon
```
bindigit := '[01_]';
decdigit := bindigit | '[23456789]';
hexdigit := decdigit | '[aAbBcCdDeEfF]';

string-char := '[^"\n]';

white-space-char := '[ \n\t\r]';

escaped-double-quote := '\\"';

// identifiers starting with '$' will be deleted after parsing of the
// configuration file has finished and can be seen as temporary variables.
identifier := '[a-zA-Z_$][a-zA-Z0-9_]*'

unsigned :=
    '0[dD]'? decdigit+ |
	'0[xX]' hexdigit+ |
	'0[bB]' bindigit+;

int := '-'? unsigned;

float :=
	int '.' unsigned ('[eE]' int)? |
	int '.' ('[eE]' int)? |
	'-'? '.' unsigned ('[eE]' int)?;

color := '#' (hexdigit{6} | hexdigit{8});

boolean := 'true' | 'false';

null := 'null';

string := '"' (string-char | escaped-double-quote)*? '"';

path := '<' .*? '>';

comment := '//.*?\n';

binary-operator := '==|!=|<=|>=|<>|and|or|not|xor|[+-*/%~&.|]';

AS := '[=:]'
SC := '[;,]';

```

## Syntax
Ignores comment and white-space-char tokens.

```
empty := ;

section-statement :=
	'[' expression ']' |		  				// Set prefix key from this point onwards the current object.
	'[]';			     						// Unset prefix key from this point onwards the current object.

assignment-statement := key AS expression;  // Replace value.

expression-statement := expression;             // Any function call will imply a method on the current object.
                                                // The returned object of this expression will be merged with the current object.

statement :=
    section-statement |
    expression-statment SC |
    assignment-statement SC;
                                               
statement-list := statement*;

object := '{' statement-list '}';

expression-list :=
    empty |
    expression (SC expression)* SC?;

array := '[' expression-list ']';

expression :=
	'(' expression ')'
	int |
	float |
	boolean |
	null |
	string |
    color |
    path |
	array |
	object;
	expression '(' expression-list ')' |
    unary-operator expression |
	expression binary-operator expression |
	expression '[' expression ']' |
	identifier |
	literal;

file := 
    empty |
    object |
    statement-list;

```

## Functions

### color(r: float, g: float, b: float, a: float=1.0)
Return a color object. Given r, g, b values are sRGB including gamma.
r, g, b values outside of the range 0.0-1.0 are allowed and are clamped to
the color space of the display.

### color(string)
Return a color object. The string is a '#' character followed by 6 or 8 hex digits
each pair denoting r, g, b, a values in sRGB color space including gamma.
