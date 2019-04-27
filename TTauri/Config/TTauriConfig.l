%option noinput nounput noyywrap 8bit nodefault                                 
%option yylineno
%option reentrant bison-bridge bison-locations
%option nounistd never-interactive
%{
#include "TTauriConfigParser.hpp"
#include "TTauri/Config/ParseContext.hpp"
#include "TTauri/Config/LiteralParser.hpp"

#define YY_DECL int yylex(YYSTYPE* yylval_param, YYLTYPE* yylloc_param, yyscan_t yyscanner, TTauri::Config::ParseContext* context)
%}

BIN             [_01]
OCT             [_0-7]
DEC             [_0-9]
HEX             [_0-9a-fA-F]

STR             [^"\n\r]
STRDQ           "\\\""
WS              [ \t\f\r\n]

%%

[=:]                        return '=';
[;,]                        return ';';

"not"                       return TTauriConfig_yyfind_token(yytext);
"<<"                        return TTauriConfig_yyfind_token(yytext);
">>"                        return TTauriConfig_yyfind_token(yytext);
"<="                        return TTauriConfig_yyfind_token(yytext);
">="                        return TTauriConfig_yyfind_token(yytext);
"=="                        return TTauriConfig_yyfind_token(yytext);
"!="                        return TTauriConfig_yyfind_token(yytext);
"and"                       return TTauriConfig_yyfind_token(yytext);
"xor"                       return TTauriConfig_yyfind_token(yytext);
"or"                        return TTauriConfig_yyfind_token(yytext);
"true"                      return TTauriConfig_yyfind_token(yytext);
"false"                     return TTauriConfig_yyfind_token(yytext);
"null"                      return TTauriConfig_yyfind_token(yytext);
[][)(}{.~*/%+<>&^|]         return yytext[0];

[_$a-zA-Z][_$a-zA-Z0-9]*    yylval->string = TTauri::Config::parseIdentifier(yytext); return T_IDENTIFIER;

-?0[bBoOdDxX]{HEX}+         yylval->integer = TTauri::Config::parseInteger(yytext); return T_INTEGER;
-?{DEC}+                    yylval->integer = TTauri::Config::parseInteger(yytext); return T_INTEGER;

-?"."{DEC}+                 yylval->real = TTauri::Config::parseFloat(yytext); return T_FLOAT;
-?{DEC}+"."{DEC}*           yylval->real = TTauri::Config::parseFloat(yytext); return T_FLOAT;

-                           return yytext[0];

\"({STR}|{STRDQ})*?\"       yylval->string = TTauri::Config::parseString(yytext); return T_STRING;

"//".*?\n                   ;
{WS}                        ;


.                           ;


%%
