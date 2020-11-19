# How Unicode is used in ttauri

## Source file character set (ASCII)
Since Visual Studio currently requires "UTF-8 with BOM" to handle UTF-8 files, we
currently require the source code to be pure ASCII.

## Execution character set (UTF-8)
This is the character set how to compiler will write string and character literals
in memory.

MSVC, gcc and clang can configure the execution character set for `char const *` to UTF-8,
which is what is used for TTauri.

This means the default encoding for `char`, `char const *` and `std::string` will be
UTF-8.

## Locale character set
When printing and reading from console, strings should be converted to/from the current
locale and UTF-8.

## Wide strings
Wide string should only be used when communicating with the win32 API.
TTauri compiles against the win32 API using UNICODE. It is recommended to explicitly
use the `W` suffix functions of win32.

win32 API will handle proper UTF-16 with surrogate pairs, there are some odd situation
in the win32 API where incomplete surrogate pairs are allowed, such as in file names.

The wide string literal execution sert for `wchar_t const *` cannot be configured
for all compilers. Therefor `wchar_t const *` string literals should not be used.

## Text formatting characters
Limitted formatting may be possible in a label to show on the display.

 ----------:| ------------ |
   U+101b00 | Text decoration none
   U+101b01 | Text decoration overline
   U+101b02 | Text decoration underline
   U+101b03 | Text decoration strikethrough
   U+101b04 | Text decoration style solid
   U+101b05 | Text decoration style double
   U+101b06 | Text decoration style dotted
   U+101b07 | Text decoration style dashed
   U+101b08 | Text decoration style wavy

   U+101b10 | Text decoration color foreground
   U+101b11 | Text decoration color accent
   U+101b17 | Text decoration color blue
   U+101b18 | Text decoration color green
   U+101b19 | Text decoration color indigo
   U+101b1a | Text decoration color orange
   U+101b1b | Text decoration color pink
   U+101b1c | Text decoration color purple
   U+101b1d | Text decoration color red
   U+101b1e | Text decoration color teal
   U+101b1f | Text decoration color yellow
    
   U+101b20 | Text color foreground
   U+101b21 | Text color accent
   U+101b27 | Text color blue
   U+101b28 | Text color green
   U+101b29 | Text color indigo
   U+101b2a | Text color orange
   U+101b2b | Text color pink
   U+101b2c | Text color purple
   U+101b2d | Text color red
   U+101b2e | Text color teal
   U+101b2f | Text color yellow

### Font size
There are 16 different font sizes following the traditional English point-sizes.

   U+101b30 | Font size 6, Nonpareil
   U+101b31 | Font size 7, Minion
   U+101b32 | Font size 8, Brevier
   U+101b33 | Font size 9, Bourgeois
   U+101b34 | Font size 10, Long Prier
   U+101b35 | Font size 11, Small Pica
   U+101b36 | Font size 12, Pica
   U+101b37 | Font size 14, English
   U+101b38 | Font size 18, Greap Primer
   U+101b39 | Font size 20, Paragon
   U+101b3a | Font size 24, Double Pica
   U+101b3b | Font size 28, Double English
   U+101b3c | Font size 36, Double Great Primer, Three-line pica
   U+101b3d | Font size 48, Canon, Four-line pica
   U+101b3e | Font size 60, Five-line pica
   U+101b3f | Font size 72, Six-line pica

### Font weight
There are 10 different font weights.

   U+101b40 | Font weight 100, Thin, Hairline
   U+101b41 | Font weight 200, Extra-Light, Ultra-Light
   U+101b42 | Font weight 300, Light
   U+101b43 | Font weight 400, Regular, Normal
   U+101b44 | Font weight 500, Medium
   U+101b45 | Font weight 600, Semi-Bold, Demi-Bold
   U+101b46 | Font weight 700, Bold
   U+101b47 | Font weight 800, Extra-Bold, Ultra-Bold
   U+101b48 | Font weight 900, Heavy, Black
   U+101b49 | Font weight 950, Extra-Black, Ultra-Black

   U+101b4a | Font upright
   U+101b4b | Font italic
   U+101b4c | Font small-caps

### Font types
   U+101b50 | Font serif
   U+101b51 | Font "Georgia"
   U+101b52 | Font "Palatino Linotype", "Palatino"
   U+101b53 | Font "Book Antique"
   U+101b54 | Font "Times New Roman", "Times"
   U+101b55 | Font "Didot"
   U+101b56 | Font "American Typewriter"
   U+101b57 | Font "noto serif"

   U+101b60 | Font sans-serif
   U+101b61 | Font "Helvetica"
   U+101b62 | Font "Arial"
   U+101b63 | Font "Impact"
   U+101b64 | Font "Charcoal"
   U+101b65 | Font "Lucida Sans Unicode", "Lucia Grande"
   U+101b66 | Font "Tahoma"
   U+101b67 | Font "Geneva"
   U+101b68 | Font "Trebuchet MS",
   U+101b69 | Font "Verdana",
   U+101b6a | Font "noto sans"

   U+101b70 | Font monospace
   U+101b71 | Font "Courier New", "Courier"
   U+101b72 | Font "Lucia Console"
   U+101b73 | Font "Monaco"
   U+101b74 | Font "Andale Mono"

   U+101b78 | Font fantasy
   U+101b79 | Font "Bradley Hand"
   U+101b7a | Font "Brush Script MT"
   U+101b7b | Font "Luminari"
   U+101b7c | Font "Comic Sans"


   U+101b5f | Font mono

