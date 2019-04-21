# Config Files
Mostly like JSON, but with extra features:

 * Including files
 * Comments
 * Patching sections.

## Examples

```
foo: 3;

bar: {
	baz: foo + 3
};

// Replace everything in bar.
bar: {
	toy: "Hello World",
	widgets: [
		{name: "light", color: "#123456"},
		{name: "dark", color: "#000000"}
	]
};

// Append something to the widgets.
bar.widgets +: {name: "append", "#976274"};

// Insert at position 0 something to the widgets.
bar.widgets +0: {name: "insert", "#9999999"};

// Remove the color from the widget with the name 'dark'.
bar.widgets[name="dark"].color-;

// Remove the widget with the name 'dark'.
bar.widgets[name="dark"]-;

// Replace colour on the widget with the name 'light'.
bar.widgets[name="light"].color: "#111111";

```

## Syntax

```
bindigit := '[01_]';
decdigit := bindigit | '[23456789]';
hexdigit := decdigit | '[aAbBcCdDeEfF]';

int :=
	'-'? '0[dD]'? +decdigit |
	'-'? '0[xX]' +hexdigit |
	'-'? '0[bB]' +bindigit;

float :=
	'-'? int '.' int |
	'-'? int '.' |
	'-'? '.' int

boolean := 'true' | 'false';

null := 'null';

char := '[^"]';

escaped-quote := '\"';

string := '"' *(char | escaped-quote) '"';

array :=
	'[' ']' |
	'[' expression *(',' expression) ','? ']';


object :=
	'{' '}' |
	'{' path ':' expression *(',' path ':' expression) ','? '}'

include := 'include' string;

value := int | float | boolean | null | string | array | object;

binary-operator := '+' | '-' | '*' | '%' | '/' | '=' | '!=';

path :=
	path '.' name |
	path '[' expression ']' |
	name;

expression :=
	'(' expression ')'
	expression binary-operator expression |
	path |
	value;

patch :=
	path ':' expression

block :=
	include |
	patch

file := *block






```
