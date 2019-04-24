
%token T_IDENTIFIER
%token T_INTEGER
%token T_FLOAT
%token T_BOOLEAN
%token T_NULL
%token T_STRING
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
%left '.' FCALL SUBSCRIPT

%start root
%%

expression:
      '(' expression ')'
    | array
    | object
    | T_INTEGER
    | T_FLOAT
    | T_BOOLEAN
    | T_NULL
    | T_STRING
    | T_IDENTIFIER
    | T_IDENTIFIER '(' expressions ')' %prec FCALL
    | T_IDENTIFIER '[' expressions ']' %prec SUBSCRIPT
    | expression '.' expression
    | '~' expression
    | T_LOG_NOT expression
    | '-' expression %prec UMINUS
    | expression '*' expression
    | expression '/' expression
    | expression '%' expression
    | expression '+' expression
    | expression '-' expression
    | expression T_SHL expression
    | expression T_SHR expression
    | expression '<' expression
    | expression '>' expression
    | expression T_LE expression
    | expression T_GE expression
    | expression T_EQ expression
    | expression T_NE expression
    | expression T_BIN_AND expression
    | expression T_BIN_XOR expression
    | expression T_BIN_OR expression
    | expression T_LOG_AND expression
    | expression T_LOG_XOR expression
    | expression T_LOG_OR expression
    ;

expressions:
      expression
    | expressions T_SC expression
    ;

array:
      '[' ']'
    | '[' expressions ']'
    | '[' expressions T_SC ']'
    ;

key:
      T_IDENTIFIER
    | key '.' T_IDENTIFIER
    ;

last_statement:
     '[' key ']'
    | key T_AS expression
    | T_IDENTIFIER '(' expressions ')'
    ;

nonlast_statement:
      '[' key ']'
    | key T_AS expression T_SC
    | T_IDENTIFIER '(' expressions ')' T_SC
    ;

nonlast_statements:
      nonlast_statement
    | nonlast_statements nonlast_statement
    ;

object:
      '{' '}'
    | '{' last_statement '}'
    | '{' nonlast_statements last_statement '}'
    ;

root:
      /* empty */
    | nonlast_statements
    ;

%%
