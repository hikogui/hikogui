
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

# Same precedence order as C++ operators.
%left '.'
%right T_BIN_NOT T_LOG_NOT
%left '*' '/' '%'
%left '+' '-'
%left T_SHL T_SHR
%left '<' '>' T_LE T_GE
%left T_EQ T_NE
%left T_BIN_AND
%left T_BIN_XOR
%left T_BIN_OR
%left T_LOG_AND
%left T_LOG_XOR
%left T_LOG_OR
%right T_AS
%left T_SC

%start root
%%

expression: '(' expression ')'
          | list

expressions: expression
           | expressions T_SC expression
           ;

list: '[' ']
    | '[' expressions ']'
    | '[' expressions T_SC ']'
    ;


object_statements: object_statement
          | object_statements object_statement
          ;

object: '{' '}'
      | '{' object_statements '}'
      | '{' object_statements T_SC '}'
      ;

root_statement: '[' key ']'
              | key T_AS expression T_SC
              | expression T_SC
              ;

root_statements:  /* empty */
               | root_statement
               | root_statements root_statement
               ;

root: /* empty */
    | root_statements
    ;

%%
