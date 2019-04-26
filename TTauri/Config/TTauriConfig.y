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
    bool boolean;
    TTauri::Config::ASTExpression *expression;
    TTauri::Config::ASTExpressions *expressions;
    TTauri::Config::ASTStatement *statement;
    TTauri::Config::ASTStatements *statements;
    TTauri::Config::ASTObject *object;
}

%token <string> T_IDENTIFIER
%token <integer> T_INTEGER
%token <real> T_FLOAT
%token <boolean> T_BOOLEAN
%token T_NULL
%token <string> T_STRING
%token T_AS
%token T_SC
%token T_LOG_NOT
%token T_SHL
%token T_SHR
%token T_LE
%token T_GE
%token T_EQ
%token T_NE
%token T_LOG_AND
%token T_LOG_XOR
%token T_LOG_OR

// Same precedence order as C++ operators.
%left T_SC
%right T_AS
%left T_LOG_OR
%left T_LOG_XOR
%left T_LOG_AND
%left T_BIN_OR
%left T_BIN_XOR
%left T_BIN_AND
%left T_EQ T_NE
%left '<' '>' T_LE T_GE
%left T_SHL T_SHR
%left '+' '-'
%left '*' '/' '%'
%right '~' T_LOG_NOT UMINUS
%left FCALL SUBSC
%left '.'

%type <object> root
%type <expression> expression
%type <expression> array
%type <expression> object
%type <expression> key
%type <expressions> expressions
%type <statement> last_statement
%type <statement> nonlast_statement
%type <statements> nonlast_statements
%start root
%%

expression:
      '(' expression ')'                                            { $$ = $2; }
    | array                                                         { $$ = $1; }
    | object                                                        { $$ = $1; }
    | T_INTEGER                                                     { $$ = NEW_NODE(ASTInteger, @1, $1); }
    | T_FLOAT                                                       { $$ = NEW_NODE(ASTFloat, @1, $1); }
    | T_BOOLEAN                                                     { $$ = NEW_NODE(ASTBoolean, @1, $1); }
    | T_NULL                                                        { $$ = NEW_NODE(ASTNull, @1); }
    | T_STRING                                                      { $$ = NEW_NODE(ASTString, @1, $1); }
    | T_IDENTIFIER                                                  { $$ = NEW_NODE(ASTName, @1, $1); }
    | T_IDENTIFIER '(' expressions ')' %prec FCALL                  { $$ = NEW_NODE(ASTCall, @4, $1, $3); }
    | expression '.' T_IDENTIFIER '(' expressions ')' %prec FCALL   { $$ = NEW_NODE(ASTCall, @4, $1, $3, $5); }
 //   | T_IDENTIFIER '[' expressions ']' %prec SUBSCRIPT
 //   | expression '.' T_IDENTIFIER '[' expressions ']' %prec SUBSC
    | expression '.' T_IDENTIFIER                                   { $$ = NEW_NODE(ASTMember, @1, $1, $3); }
    | '~' expression                                                { $$ = NEW_NODE(ASTCall, @1, $2, strdup("__bin_not__")); }
    | T_LOG_NOT expression                                          { $$ = NEW_NODE(ASTCall, @1, $2, strdup("__log_not__")); }
    | '-' expression %prec UMINUS                                   { $$ = NEW_NODE(ASTCall, @1, $2, strdup("__neg__")); }
    | expression '*' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__mul__"), $3); }
    | expression '/' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__div__"), $3); }
    | expression '%' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__mod__"), $3); }
    | expression '+' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__add__"), $3); }
    | expression '-' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__sub__"), $3); }
    | expression T_SHL expression                                   { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__shl__"), $3); }
    | expression T_SHR expression                                   { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__shr__"), $3); }
    | expression '<' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__lt__"), $3); }
    | expression '>' expression                                     { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__gt__"), $3); }
    | expression T_LE expression                                    { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__le__"), $3); }
    | expression T_GE expression                                    { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__ge__"), $3); }
    | expression T_EQ expression                                    { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__eq__"), $3); }
    | expression T_NE expression                                    { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__ne__"), $3); }
    | expression T_BIN_AND expression                               { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__bin_and__"), $3); }
    | expression T_BIN_XOR expression                               { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__bin_xor__"), $3); }
    | expression T_BIN_OR expression                                { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__bin_or__"), $3); }
    | expression T_LOG_AND expression                               { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__log_and__"), $3); }
    | expression T_LOG_XOR expression                               { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__log_xor__"), $3); }
    | expression T_LOG_OR expression                                { $$ = NEW_NODE(ASTCall, @2, $1, strdup("__log_or__"), $3); }
    ;

expressions:
      expression                                                    { $$ = NEW_NODE(ASTExpressions, @1, $1); }
    | expressions T_SC expression                                   { $1->expressions.push_back($3); $$ = $1; }
    ;

array:
      '[' ']'                                                       { $$ = NEW_NODE(ASTArray, @1); }
    | '[' expressions ']'                                           { $$ = NEW_NODE(ASTArray, @1, $2); }
    | '[' expressions T_SC ']'                                      { $$ = NEW_NODE(ASTArray, @1, $2); }
    ;

key:
      T_IDENTIFIER                                                  { $$ = NEW_NODE(ASTName, @1, $1); }
    | key '.' T_IDENTIFIER                                          { $$ = NEW_NODE(ASTMember, @2, $1, $3); }
    ;

last_statement:
     '[' key ']'                                                    { $$ = NEW_NODE(ASTPrefix, @1, $2); }
    | key T_AS expression                                           { $$ = NEW_NODE(ASTAssignment, @2, $1, $3); }
    | T_IDENTIFIER '(' expressions ')'                              { $$ = NEW_NODE(ASTExpressionStatement, @1, NEW_NODE(ASTCall, @2, $1, $3)); }
    ;

nonlast_statement:
      '[' key ']'                                                   { $$ = NEW_NODE(ASTPrefix, @1, $2); }
    | key T_AS expression T_SC                                      { $$ = NEW_NODE(ASTAssignment, @2, $1, $3); }
    | T_IDENTIFIER '(' expressions ')' T_SC                         { $$ = NEW_NODE(ASTExpressionStatement, @1, NEW_NODE(ASTCall, @2, $1, $3)); }
    ;

nonlast_statements:
      nonlast_statement                                             { $$ = NEW_NODE(ASTStatements, @1, $1); }
    | nonlast_statements nonlast_statement                          { $1->statements.push_back($2); $$ = $1; }
    ;

object:
      '{' '}'                                                       { $$ = NEW_NODE(ASTObject, @1); }
    | '{' last_statement '}'                                        { $$ = NEW_NODE(ASTObject, @1, $2); }
    | '{' nonlast_statements last_statement '}'                     { $2->statements.push_back($3); $$ = NEW_NODE(ASTObject, @1, $2); }
    ;

root:
      /* empty */                                                   { $$ = new TTauri::Config::ASTObject({0,0,0,0}); context->object = $$; }
    | nonlast_statements                                            { $$ = NEW_NODE(ASTObject, @1, $1); context->object = $$; }
    ;

%%
