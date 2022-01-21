Bidirectional Text Editing
==========================


Bidirectional text editing is a complex topic and it seems there are many ways
of implementing it, all of them flawed in some ways.

The way we try to solve this in ttauri is that the cursor keeps track of both the character
and if it is before or after this character. For example in the string "ab" There are four
different cursor positions:
 - before 'a'
 - after 'a'
 - before 'b'
 - after 'b'


Use cases: curses in mixed bidi text
------------------------------------

 - lower-case: left-to-right text
 - upper-case: right-to-left text
 - '>' After selected character
 - '<' Before selected character
 - ':' secondary cursor

### Paragraph direction L
```
 <a  b  D  C :             <a  b  C  D 
  a> b  D  C                a> b  C  D 
  a <b  D  C                a <b  C  D 
  a  b> D  C:               a  b> C  D 
  a  b <D  C:               a  b  C  D>
  a  b  D> C                a  b  C <D 
  a  b  D <C                a  b  C> D 
  a  b: D  C>               a  b >C  D 

 <a  b  D  C  e  f :            <a  b  C  D  e  f
  a> b  D  C  e  f               a> b  C  D  e  f
  a <b  D  C  e  f               a <b  C  D  e  f
  a  b> D  C: e  f               a  b> C  D  e  f
  a  b <D  C: e  f               a  b  C  D> e  f
  a  b  D> C  e  f               a  b  C <D  e  f
  a  b  D <C  e  f               a  b  C> D  e  f
  a  b: D  C> e  f               a  b >C  D  e  f
  a  b: D  C <e  f               a  b  C  D >e  f
  a  b  D  C  e> f               a  b  C  D  e> f
  a  b  D  C  e <f               a  b  C  D  e <f
  a  b  D  C  e  f>              a  b  C  D  e  f>
```

### Paragraph direction R
```
: c  d  B  A>             <A  B  c  d 
  c  d  B <A               A> B  c  d 
  c  d  B> A               A <B  c  d 
 :c  d <B  A               A  B> c  d 
 :c  d> B  A               A  B  c  d>
  c <d  B  A               A  B  c <d 
  c> d  B  A               A  B  c> d 
 <c  d: B  A               A  B <c  d 

: F  E  c  d  B  A>             <A  B  c  d  E  F
  D  E  c  d  B <A               A> B  c  d  E  F
  D  E  c  d  B> A               A <B  c  d  E  F
  F  E: c  d <B  A               A  B> c  d  E  F
  F  E: c  d> B  A               A  B  c  d> E  F
  F  E  c <d  B  A               A  B  c <d  E  F
  F  E  c> d  B  A               A  B  c> d  E  F
  F  E <c  d: B  A               A  B <c  d  E  F
  F  E> c  d: B  A               A  B  c  d <E  F
  F <E  c  d  B  A               A  B  c  d  E> F
  F> E  c  d  B  A               A  B  c  d  E <F
 <F  E  c  d  B  A               A  B  c  d  E  F>
```

Cursor encoding
---------------
To be able to figure out what the primary cursor is we need to
keep track of the character that was selected or last modified; and
on which side of the character the cursor is.

Encode as an unsigned integer plus a before/after bit.

If the text is empty then the cursor is 0:before.


