Bidirectional Text Editing
==========================

 - lower-case: left-to-right text
 - upper-case: right-to-left text
 - '#' neutral character.
 - '>' selected character, cursor to right
 - '<' selected character, cursor to left
 - ')' other cursor to right
 - '(' other cursor to left
 - '|' primary cursor
 - ':' secondary cursor


```
 >helloDLROW    >helloWORLD    BS=  DEL=h
 <helloDLROW    <helloWORLD    BS=  DEL=h
 he>lloDLROW    he>lloWORLD    BS=e DEL=l
 he<lloDLROW    he<lloWORLD    BS=e DEL=l
 hello>DLROW:   hello>WORLD    BS=o DEL=
 hello<DLROW:   helloWORLD<    BS=D DEL=
 helloDL>ROW    helloWOR>LD    BS=R DEL=L
 helloDL<ROW    helloWOR<LD    BS=R DEL=L
 hello:DLROW>   hello>WORLD    BS=  DEL=W
 helloDLROW<    hello<WORLD:   BS=  DEL=W
```

```
 >OLLEHworld    :HELLO|world
 <OLLEH:world   HELLO|:world
 OL>LEHworld    HEL|:LOworld
 OL<LEHworld    HEL|:LOworld
 OLLEH>world    |HELLO:world
 OLLEH<world    :HELLO|world
 OLLEHwo>rld    HELLOwo:|rld
 OLLEHwo<rld    HELLOwo:|rld
 OLLEHworld>    HELLOworld:|
 OLLEHworld<    HELLOworld|:
```







