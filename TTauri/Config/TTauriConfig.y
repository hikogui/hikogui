%define api.pure full
%define parse.lac full
%define parse.error verbose
%locations
%param { yyscan_t scanner }
%param { TTauri::Config::ParseContext* context }

%code top {
  #include <stdio.h>

  #define NEW_NODE(ast_type, location, ...) new TTauri::Config::ast_type({location.first_line, location.last_line, location.first_column, location.last_column}, __VA_ARGS__)
} 
%code requires {
  #include <string>
  #include <cstdint>
  #include <memory>
  #include "TTauri/Config/AST.hpp"
  #include "TTauri/Config/ParseContext.hpp"
  typedef void* yyscan_t;
  int TTauriConfig_yyfind_token(char *text);
}

%code {
  int yylex(YYSTYPE* yylvalp, YYLTYPE* yyllocp, yyscan_t scanner, TTauri::Config::ParseContext* context);
  void yyerror(YYLTYPE* yyllocp, yyscan_t scanner, TTauri::Config::ParseContext* context, const char* msg) {
    context->setError({yyllocp->first_line, yyllocp->last_line, yyllocp->first_column, yyllocp->last_column}, msg);
  }
}

%union {
    char *string;
    int64_t integer;
    double real;
    TTauri::Config::ASTExpression *expression;
    TTauri::Config::ASTExpressions *expressions;
    TTauri::Config::ASTObject *object;
    TTauri::Config::ASTArray *array;
}

%token-table
%token <string> T_IDENTIFIER
%token <integer> T_INTEGER
%token <real> T_FLOAT
%token <string> T_STRING

// Same precedence order as C++ operators.
%left ';'
%right '='
%left "or"
%left "xor"
%left "and"
%left '|'
%left '^'
%left '&'
%left "==" "!="
%left '<' '>' "<=" ">="
%left "<<" ">>"
%left '+' '-'
%left '*' '/' '%'
%right '~' "not" UMINUS
%left '(' '['
%left '.'

%type <object> root
%type <expression> expression
%type <object> object
%type <array> array
%type <expressions> expressions
%type <expressions> statements
%start root
%%

array:
      '[' ']'                                                       { $$ = NEW_NODE(ASTArray, @1); }
    | '[' expressions ']'                                           { $$ = NEW_NODE(ASTArray, @1, $2); }
    | '[' expressions ';' ']'                                       { $$ = NEW_NODE(ASTArray, @1, $2); }
    ;

object:
      '{' '}'                                                       { $$ = NEW_NODE(ASTObject, @1); }
    | '{' expressions '}'                                           { $$ = NEW_NODE(ASTObject, @1, $2); }
    | '{' expressions ';' '}'                                       { $$ = NEW_NODE(ASTObject, @1, $2); }
    ;

expression:
      '(' expression ')'                                            { $$ = $2; }
    | object                                                        { $$ = $1; }
    | array                                                         { $$ = $1; }
    | T_INTEGER                                                     { $$ = NEW_NODE(ASTInteger, @1, $1); }
    | T_FLOAT                                                       { $$ = NEW_NODE(ASTFloat, @1, $1); }
    | "true"                                                        { $$ = NEW_NODE(ASTBoolean, @1, true); }
    | "false"                                                       { $$ = NEW_NODE(ASTBoolean, @1, false); }
    | "null"                                                        { $$ = NEW_NODE(ASTNull, @1); }
    | T_STRING                                                      { $$ = NEW_NODE(ASTString, @1, $1); }
    | T_IDENTIFIER                                                  { $$ = NEW_NODE(ASTName, @1, $1); }
    | expression '=' expression                                     { $$ = NEW_NODE(ASTAssignment, @2, $1, $3); }
    | expression '(' expressions ')'                                { $$ = NEW_NODE(ASTCall, @2, $1, $3); }
    | expression '[' expressions ']'                                { $$ = NEW_NODE(ASTSlice, @2, $1, $3); }
    | expression '.' T_IDENTIFIER                                   { $$ = NEW_NODE(ASTMember, @1, $1, $3); }
    | '~' expression                                                { $$ = NEW_NODE(ASTCall, @1, $2, strdup("__not__")); }
    | '-' expression %prec UMINUS                                   { $$ = NEW_NODE(ASTCall, @1, $2, strdup("__neg__")); }
    | expression '*' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__mul__"), $3); }
    | expression '/' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__div__"), $3); }
    | expression '%' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__mod__"), $3); }
    | expression '+' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__add__"), $3); }
    | expression '-' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__sub__"), $3); }
    | expression "<<" expression                                    { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__shl__"), $3); }
    | expression ">>" expression                                    { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__shr__"), $3); }
    | expression '<' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__lt__"), $3); }
    | expression '>' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__gt__"), $3); }
    | expression "<=" expression                                    { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__le__"), $3); }
    | expression ">=" expression                                    { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__ge__"), $3); }
    | expression "==" expression                                    { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__eq__"), $3); }
    | expression "!=" expression                                    { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__ne__"), $3); }
    | expression '&' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__and__"), $3); }
    | expression '^' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__xor__"), $3); }
    | expression '|' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__or__"), $3); }
    | "not" expression                                              { $$ = NEW_NODE(ASTCall, @1, $2, strdup("__logic_not__")); }
    | expression "and" expression                                   { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__logic_and__"), $3); }
    | expression "xor" expression                                   { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__logic_xor__"), $3); }
    | expression "or" expression                                    { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__logic_or__"), $3); }
    ;

expressions:
      expression                                                    { $$ = NEW_NODE(ASTExpressions, @1, $1); }
    | expressions ';' expression                                    { $1->expressions.push_back($3); $$ = $1; }
    ;

statements:
      expression ';'                                                { $$ = NEW_NODE(ASTExpressions, @1, $1); }
    | statements expression ';'                                     { $1->expressions.push_back($2); $$ = $1; }
    ;

root:
      /* empty */                                                   { $$ = new TTauri::Config::ASTObject({0,0,0,0}); context->object = $$; }
    | object                                                        { $$ = $1; context->object = $$; };
    | statements                                                    { $$ = NEW_NODE(ASTObject, @1, $1); context->object = $$; }
    ;

%%

int TTauriConfig_yyfind_token(char *text)
{
    size_t size = strlen(text);

    for (size_t i = 0; i < YYNTOKENS; i++) {
        if (
            (yytname[i] != 0) &&
            (yytname[i][0] == '"') &&
            (strncmp (yytname[i] + 1, text, size) == 0) &&
            (yytname[i][size + 1] == '"') &&
            (yytname[i][size + 2] == 0)
        ) {
            return i;
        }
    }
    return 1;
}
