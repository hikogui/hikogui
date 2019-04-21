# Config Files
Mostly like JSON, but with extra features:

 * Including files
 * Comments
 * Patching sections.

## Examples

```


```


## Lexicon
```
bindigit := '[01_]';
decdigit := bindigit | '[23456789]';
hexdigit := decdigit | '[aAbBcCdDeEfF]';

string-char := '[^"]';

white-space-char := '[ \n\t\r]';

escaped-double-quote := '\\"';

// identifiers starting with '$' will be deleted after parsing of the
// configuration file has finished and can be seen as temporary variables.
identifier := '[a-zA-Z_$][a-zA-Z0-9_]*'

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

delete := 'delete';

string := '"' *(string-char | escaped-double-quote) '"';

color := '#' (hexdigit{6} | hexdigit{8});

comment := '//.*?\n';

keywords := 'include';

```

## Syntax
Ignores comment and white-space-char tokens.

```
array :=
	'[' ']' |
	'[' expression *(',' expression) ?',' ']';

statement :=
	'[' key ']' |		    					// Set prefix key from this point onwards in this object.
	'[]' |			     						// Unset prefix key from this point onwards in this object.
	key '-' [,;] |								// Delete key.
	key ':' expression [,;] |					// Replace value.
	key ':@' expression [,;] |					// Replace last value in list.
	key ':' expression '@' expression [,;] |	// Replace value at index in list. Negative index from end of list.
	key ':-' ?[,;] |							// Delete last entry from list.
	key ':' expression '-' ?[,;] |				// Delete at index. Negative index from end of list.
	key ':+' expression ?[,;] |					// Append value at end of list.
	key ':' expression '+' expression ?[,;] |	// Insert value before index. Negative index from end of list.
	'include' '(' expression ')' ?[,;];			// Object included from another file is merged with this object.

object := '{' *statement '}';

literal :=
	int |
	float |
	boolean |
	null |
	string |
	color |
	array |
	object;

binary-operator := '+' | '-' | '*' | '%' | '/' | '=' | '!=';

key :=
	key '.' identifier |
	identifier;

expression :=
	'(' expression ')'
	'include' '(' expression ')' |
	expression '(' ')' |
	expression '(' expression *(',' expression) ?',' ')' |
	expression binary-operator expression |
	expression '.' identifier |
	expression '[' expression ']' |
	identifier |
	literal;

file := *statement;

```
