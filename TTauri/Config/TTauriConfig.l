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

[_$a-zA-Z][_$a-zA-Z0-9]*    yylval->string = strdup(yytext); return T_IDENTIFIER;

-0[bB]{BIN}+                yylval->integer = TTauri::Config::parseInteger(&yytext[3], 2, true); return T_INTEGER;
-0[oO]{OCT}+                yylval->integer = TTauri::Config::parseInteger(&yytext[3], 8, true); return T_INTEGER;
-0[dD]{DEC}+                yylval->integer = TTauri::Config::parseInteger(&yytext[3], 10, true); return T_INTEGER;
-0[xX]{HEX}+                yylval->integer = TTauri::Config::parseInteger(&yytext[3], 16, true); return T_INTEGER;
0[bB]{BIN}+                 yylval->integer = TTauri::Config::parseInteger(&yytext[2], 2, false); return T_INTEGER;
0[oO]{OCT}+                 yylval->integer = TTauri::Config::parseInteger(&yytext[2], 8, false); return T_INTEGER;
0[dD]{DEC}+                 yylval->integer = TTauri::Config::parseInteger(&yytext[2], 10, false); return T_INTEGER;
0[xX]{HEX}+                 yylval->integer = TTauri::Config::parseInteger(&yytext[2], 16, false); return T_INTEGER;
-{DEC}+                     yylval->integer = TTauri::Config::parseInteger(&yytext[1], 10, true); return T_INTEGER;
{DEC}+                      yylval->integer = TTauri::Config::parseInteger(yytext, 10, false); return T_INTEGER;
#{HEX}{6}                   yylval->integer = (TTauri::Config::parseInteger(&yytext[1], 16, false) << 8) | 0xff; return T_COLOR;
#{HEX}{8}                   yylval->integer = TTauri::Config::parseInteger(&yytext[1], 16, false); return T_COLOR;

-?"."{DEC}+                 yylval->real = TTauri::Config::parseFloat(yytext); return T_FLOAT;
-?{DEC}+"."{DEC}*           yylval->real = TTauri::Config::parseFloat(yytext); return T_FLOAT;

\"({STR}|{STRDQ})*?\"       yylval->string = TTauri::Config::parseString(yytext); return T_STRING;
<.*?>                       yylval->string = strndup(&yytext[1], strlen(yytext) - 2); return T_PATH;

[][)(}{.~*/%+<>&^|-]        return yytext[0];


"//".*?\n                   ; // Comment is ignored.
{WS}                        ; // Whitespace is ignored.

.                           ;

%%
