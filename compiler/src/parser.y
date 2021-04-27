%{
#define YYDEBUG 1
#include <cstdio>
#include <string>
#include "lexer.hpp"
%}


%token T_and                "and"
%token T_array              "array"
%token T_begin              "begin"
%token T_bool               "bool"
%token T_char               "char"
%token T_delete             "delete"
%token T_dim                "dim"
%token T_do                 "do"
%token T_done               "done"
%token T_downto             "downto"
%token T_else               "else"
%token T_end                "end"
%token T_false              "false"
%token T_float              "float"
%token T_for                "for"
%token T_if                 "if"
%token T_in                 "in"
%token T_int                "int"
%token T_let                "let"
%token T_match              "match"
%token T_mod                "mod"
%token T_mutable            "mutable"
%token T_new                "new"
%token T_not                "not"
%token T_of                 "of"
%token T_rec                "rec"
%token T_ref                "ref"
%token T_then               "then"
%token T_to                 "to"
%token T_true               "true"
%token T_type               "type"
%token T_unit               "unit"
%token T_while              "while"
%token T_with               "with"

%token T_op_arrow           "->"
%token T_op_fadd            "+."
%token T_op_fsub            "-."
%token T_op_fmul            "*."
%token T_op_fdiv            "/."
%token T_op_fpow            "**"
%token T_op_and             "&&"
%token T_op_or              "||"
%token T_op_struct_neq      "<>"
%token T_op_leq             "<="
%token T_op_geq             ">="
%token T_op_eq              "=="
%token T_op_phys_neq        "!="
%token T_op_assign          ":="

%token<::std::string> T_id            "id"
%token<::std::string> T_constructor   "Id"
%token<num> T_const                   "int_const"
%token<flt> T_const_float             "float_const"
%token<::std::string> T_const_string  "string_literal"
%token<chr> T_const_char              "char_const"

/* Priority and associativity of Llama operators. */
/* each line in this list is a level of priority
   each line in this list has higher priority 
   than the line before
*/

%precedence LETIN
%left ';'
%precedence "then"
%precedence "else"
%nonassoc ":="
%left "||"
%left "&&"
%nonassoc '=' "<>" '>' '<' "<=" ">=" "==" "!="
%left '+' '-' "+." "-."
%left '*' '/' "*." "/." "mod"
%right "**"
%precedence "not" "delete" SIGN
// %nonassoc '!' 
// %nonassoc "new"

%right "->"
%precedence "of" // possible terminal 
%precedence "ref"

%union {
    AST *ast;
    int num;
    float flt;
    char chr;
}

%type<ast> letdef def typedef tdef constr par 

%%

program: 
    stmt_list {
        std::cout << "AST: " << *$1 << std::endl;
    }
;

stmt_list: %empty  
    /* nothing */                                               {  }
|   letdef stmt_list                                            {  }
|   typedef stmt_list                                           {  }
;

letdef:
    "let" def def_gen                                           { $$ = new Let($2, $3, false); }
|   "let" "rec" def def_gen                                     { $$ = new Let($3, $4, true); }
;

def_gen: %empty  
    /* nothing */                                               { $$ = nullptr; }
|   "and" def def_gen                                           { $$ = new DefGen($2, $3); }
;

def: 
    "id" par_gen '=' expr                                       { $$ = new Def($1, $2, $4, nullptr, nullptr, false); }
|   "id" par_gen ':' type '=' expr                              { $$ = new Def($1, $2, $6, $4, nullptr, false); }
|   "mutable" "id"                                              { $$ = new Def($2, nullptr, nullptr, nullptr, nullptr, true); }
|   "mutable" "id" ':' type                                     { $$ = new Def($2, nullptr, nullptr, $4, nullptr, true); }
|   "mutable" "id" '[' expr comma_expr_gen ']'                  { $$ = new Def($2, nullptr, $4, nullptr, $5, true); }
|   "mutable" "id" '[' expr comma_expr_gen ']' ':' type         { $$ = new Def($2, nullptr, $4, $8, $5, true); }
;

par_gen: %empty
    /* nothing */                                               { $$ = nullptr; }
|   par par_gen;                                                { $$ = new ParGen($1, $2); }

comma_expr_gen: %empty
    /* nothing */                                               { $$ = nullptr; }
|   ',' expr comma_expr_gen                                     { $$ = new CommaExprGen($2, $3); }
;

typedef: 
    "type" tdef tdef_gen                                        { $$ = new TypeDef($2, $3); }
;

tdef_gen: %empty 
    /* nothing */                                               {  }
|   "and" tdef tdef_gen                                         {  }
;

tdef: 
    "id" '=' constr bar_constr_gen                              {  }
;

bar_constr_gen: %empty 
    /* nothing */                                               {  }
|   '|' constr bar_constr_gen                                   {  }
;

constr: 
    "Id"                                                        {  }
|   "Id" "of" type_gen                                          {  }
;

type_gen: 
    type                                                        {  }
|   type type_gen                                               {  }
;                                              

par:
    "id"                                                        {  }
|   '(' "id" ':' type ')'                                       {  }
;

type: 
    "unit"                                                      {  }
|   "int"                                                       {  }
|   "char"                                                      {  }
|   "bool"                                                      {  }
|   "float"                                                     {  }
|   '(' type ')'                                                {  }
|   type "->" type                                              {  }
|   type "ref"                                                  {  }
|   "array" "of" type                                           {  }
|   "array" '[' comma_star_gen ']' "of" type                    {  }
|   "id"                                                        {  }
;

comma_star_gen: 
    '*'                                                         {  }
|   '*' ',' comma_star_gen                                      {  }
;

expr_high:
    '!' expr_high                                               {  }
|   '(' expr ')'                                                {  }
|   "int_const"                                                 {  }
|   "float_const"                                               {  }
|   "char_const"                                                {  }
|   "string_literal"                                            {  }
|   "true"                                                      {  }
|   "false"                                                     {  }
|   '(' ')'                                                     {  }
|   "id" '[' expr comma_expr_gen ']'                            {  }
|   "id"                                                        {  }
|   "Id"                                                        {  }
;

expr:
    '+' expr %prec SIGN                                         {  }
|   '-' expr %prec SIGN                                         {  }
|   "+." expr %prec SIGN                                        {  }
|   "-." expr %prec SIGN                                        {  }
|   "not" expr                                                  {  }
|   expr '+' expr                                               {  }
|   expr '-' expr                                               {  }
|   expr '*' expr                                               {  }
|   expr '/' expr                                               {  }
|   expr "+." expr                                              {  }
|   expr "-." expr                                              {  }
|   expr "*." expr                                              {  }
|   expr "/." expr                                              {  }
|   expr "mod" expr                                             {  }
|   expr "**" expr                                              {  }
|   expr '=' expr                                               {  }
|   expr "<>" expr                                              {  }
|   expr '<' expr                                               {  }
|   expr '>' expr                                               {  }
|   expr "<=" expr                                              {  }
|   expr ">=" expr                                              {  }
|   expr "==" expr                                              {  }
|   expr "!=" expr                                              {  }
|   expr "&&" expr                                              {  }
|   expr "||" expr                                              {  }
|   expr ';' expr                                               {  }
|   expr ":=" expr                                              {  }
|   "id" expr_high expr_high_gen                                {  }
|   "Id" expr_high expr_high_gen                                {  }
|   "dim" "id"                                                  {  }
|   "dim" "int_const" "id"                                      {  }
|   "new" type                                                  {  }
|   "delete" expr                                               {  }
|   letdef "in" expr %prec LETIN                                {  }
|   "begin" expr "end" /* possible to go to expr_high */        {  }
|   "if" expr "then" expr                                       {  }
|   "if" expr "then" expr "else" expr                           {  }
|   "while" expr "do" expr "done"                               {  }
|   "for" "id" '=' expr "to" expr "do" expr "done"              {  }
|   "for" "id" '=' expr "downto" expr "do" expr "done"          {  }
|   "match" expr "with" clause bar_clause_gen "end"             {  }
|   expr_high                                                   {  }
;

expr_high_gen: %empty
    /* nothing */                                               {  }
|   expr_high expr_high_gen                                     {  }
;

bar_clause_gen: %empty 
    /* nothing */                                               {  }
|   '|' clause bar_clause_gen                                   {  }
;

clause: 
    pattern "->" expr                                           {  }
;

pattern:
    pattern_high                                                {  }
|   "Id" pattern_high_gen                                       {  }
;

pattern_high:
    '+' "int_const" %prec SIGN                                  {  }
|   '-' "int_const" %prec SIGN                                  {  }
|   "+." "float_const" %prec SIGN                               {  }
|   "-." "float_const" %prec SIGN                               {  }
|   "char_const"                                                {  }
|   "true"                                                      {  }
|   "false"                                                     {  }
|   "id"                                                        {  }
|   '(' pattern ')'                                             {  }
;

pattern_high_gen: %empty
    /* nothing */                                               {  }
|   pattern_high pattern_high_gen                               {  }
;

%%

int main() {
  yydebug = 1;
  int result = yyparse();
  if (result == 0) printf("Success.\n");
  return result;
}
