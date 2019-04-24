
%{
#include "TTauriConfig.tab.h"
%}

BIN             [_01]
OCT             [_0-7]
DEC             [_0-9]
HEX             [_0-9a-fA-F]

STR             [^"\n\r]
STRDQ           "\\\""
WS              [ \t\f\r\n]

%%

[_$a-zA-Z][_$a-zA-Z0-9]*    yylval = parseIdentifier(yytext); return T_IDENTIFIER;

-?0[bBoOdDxX]{HEX}+         yylval = parseInteger(yytext); return T_INTEGER;
-?{DEC}+                    yylval = parseInteger(yytext); return T_INTEGER;

-?"."{DEC}+                 yylval = parseFloat(yytext); return T_FLOAT;
-?{DEC}+"."{DEC}*           yylval = parseFloat(yytext); return T_FLOAT;

true|false                  yylval = parseBoolean(yytext); return T_BOOLEAN;
null                        yylval = parseNull(yytext); return T_NULL;

\"({STR}|{STRDQ})*?\"       yylval = parseString(yytext); return T_STRING;

//.*?\n                     ;
{WS}                        ;

[=:]                        return T_AS;
[;,]                        return T_SC;

"not"                       return T_LOG_NOT;
"<<"                        return T_SHL;
">>"                        return T_SHR;
"<="                        return T_LE;
">="                        return T_GE;
"=="                        return T_EQ;
"!="                        return T_NE;
"and"                       return T_LOG_AND;
"xor"                       return T_LOG_XOR;
"or"                        return T_LOG_OR;
[][)(}{.~*/%+-<>&^|]        return *yytext;;

.                           yylval = tokenError(yytext); return T_ERROR;


%%

