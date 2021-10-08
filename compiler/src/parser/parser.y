%{
#define YYDEBUG 1
#include <cstdio>
#include <string>
#include "../ast/ast.hpp"
#include "../lexer/lexer.hpp"
#include "../optionsMenu/optionsMenu.hpp"

void appendLocInfo(AST *ast, YYLTYPE token) {
    ast->YYLTYPE.first_line = token.first_line;
    ast->YYLTYPE.last_line = token.last_line;
    ast->YYLTYPE.first_column = token.first_column;
    ast->YYLTYPE.last_column = token.last_column;
}

void changeEnd(AST *ast, YYLTYPE token) {
    ast->YYLTYPE.last_line = token.last_line;
    ast->YYLTYPE.last_column = token.last_column;
}

SymbolTable st;
PseudoSymbolTable pseudoST;

extern int yylineno;

%}
%locations

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

%token<ids> T_id                      "id" 
%token<constructor> T_constructor     "Id"
%token<num> T_const                   "int_const"
%token<flt> T_const_float             "float_const"
%token<str> T_const_string            "string_literal"
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
    
    /* for tokens */
    int num;
    float flt;
    char *chr;
    char *str;
    char *constructor;
    char *ids;
    
    int comma_star_gen;
    AST *program;
    Block *stmt_list;
    Let *letdef;
    DefGen *def_gen;
    Def *def;
    ParGen *par_gen;
    CommaExprGen *comma_expr_gen;
    TypeDef *type_def;
    TdefGen *tdef_gen;
    Tdef *tdef;
    BarConstrGen *bar_constr_gen;
    Constr *constr;
    TypeGen *type_gen;
    Par *par;
    CustomType *type;
    //*comma_star_gen;
    Expr *expr_high;
    Expr *expr;
    ExprGen *expr_high_gen;
    BarClauseGen *bar_clause_gen;
    Clause *clause;
    Pattern *pattern;
    Pattern *pattern_high;
    PatternGen *pattern_high_gen;
}

%type<program> program
%type<stmt_list> stmt_list
%type<letdef> letdef
%type<def_gen> def_gen
%type<def> def
%type<par_gen> par_gen
%type<comma_expr_gen> comma_expr_gen
%type<type_def> type_def
%type<tdef_gen> tdef_gen
%type<tdef> tdef
%type<bar_constr_gen> bar_constr_gen
%type<constr> constr
%type<type_gen> type_gen
%type<par> par
%type<type> type
%type<comma_star_gen> comma_star_gen
%type<expr_high> expr_high
%type<expr> expr
%type<expr_high_gen> expr_high_gen
%type<bar_clause_gen> bar_clause_gen
%type<clause> clause
%type<pattern> pattern
%type<pattern_high> pattern_high
%type<pattern_high_gen> pattern_high_gen

%%

program: 
    stmt_list {
        $$ = $1;
        optionsMenu->setStmtList($$);
        // std::cout << "AST: " << *$1 << std::endl;
        // $1->sem();
        // std::cout << " --- SEM COMPLETED ---\n"; std::cout.flush();
        // // pseudoST.printST();
        // if (!semError) $1->llvm_compile_and_dump();
    }
;

stmt_list: %empty  
    /* nothing */                                               { $$ = new Block(); } // new scope
|   type_def stmt_list                                          { $2->appendBlock($1); appendLocInfo($1, @1); $$ = $2; }
|   letdef stmt_list                                            { $2->appendBlock($1); appendLocInfo($1, @1); $$ = $2; }
;

letdef:
    "let" "rec" def def_gen                                     { $$ = new Let($3, $4, true);  appendLocInfo($$, @1); }
|   "let" def def_gen                                           { $$ = new Let($2, $3, false); appendLocInfo($$, @1); }
;

def_gen: %empty  
    /* nothing */                                               { $$ = nullptr; }
|   "and" def def_gen                                           { $$ = new DefGen($2, $3); appendLocInfo($$, @1); }
;

def: 
    "id" par_gen '=' expr                                       { $$ = new Def($1, $2, $4, nullptr, nullptr, false); appendLocInfo($$, @1); }
|   "id" par_gen ':' type '=' expr                              { $$ = new Def($1, $2, $6, $4, nullptr, false); appendLocInfo($$, @1); }
|   "mutable" "id"                                              { $$ = new Def($2, nullptr, nullptr, nullptr, nullptr, true); appendLocInfo($$, @2); }
|   "mutable" "id" ':' type                                     { $$ = new Def($2, nullptr, nullptr, $4, nullptr, true); appendLocInfo($$, @2); }
|   "mutable" "id" '[' expr comma_expr_gen ']'                  { $$ = new Def($2, nullptr, $4, nullptr, $5, true); appendLocInfo($$, @2); }
|   "mutable" "id" '[' expr comma_expr_gen ']' ':' type         { $$ = new Def($2, nullptr, $4, $8, $5, true); appendLocInfo($$, @2); }
;

par_gen: %empty
    /* nothing */                                               { $$ = nullptr; }
|   par par_gen                                                 { $$ = new ParGen($1, $2); appendLocInfo($$, @1); }
;

comma_expr_gen: %empty
    /* nothing */                                               { $$ = nullptr; }
|   ',' expr comma_expr_gen                                     { $$ = new CommaExprGen($2, $3); appendLocInfo($$, @1); }
;

type_def:
    "type" tdef tdef_gen                                        { $$ = new TypeDef($2, $3); appendLocInfo($$, @1); }
;

tdef_gen: %empty 
    /* nothing */                                               { $$ = nullptr; }
|   "and" tdef tdef_gen                                         { $$ = new TdefGen($2, $3); appendLocInfo($$, @1); }
;

tdef: 
    "id" '=' constr bar_constr_gen                              { $$ = new Tdef($1, $3, $4); appendLocInfo($$, @1); }
;

bar_constr_gen: %empty 
    /* nothing */                                               { $$ = nullptr; }
|   '|' constr bar_constr_gen                                   { $$ = new BarConstrGen($2, $3); appendLocInfo($$, @1); }
;

constr: 
    "Id"                                                        { $$ = new Constr($1, nullptr); appendLocInfo($$, @1); }
|   "Id" "of" type_gen                                          { $$ = new Constr($1, $3); appendLocInfo($$, @1); }
;

type_gen: 
    type                                                        { $$ = new TypeGen($1, nullptr); appendLocInfo($$, @1); }
|   type type_gen                                               { $$ = new TypeGen($1, $2); appendLocInfo($$, @1); }
;                                              

par:
    "id"                                                        { $$ = new Par($1, nullptr); appendLocInfo($$, @1); }
|   '(' "id" ':' type ')'                                       { $$ = new Par($2, $4); appendLocInfo($$, @2); }
;

type: 
    "unit"                                                      { $$ = new Unit(); appendLocInfo($$, @1); }
|   "int"                                                       { $$ = new Integer(); appendLocInfo($$, @1); }
|   "char"                                                      { $$ = new Character(); appendLocInfo($$, @1); }
|   "bool"                                                      { $$ = new Boolean(); appendLocInfo($$, @1); }
|   "float"                                                     { $$ = new Float(); appendLocInfo($$, @1); }
|   '(' type ')'                                                { $$ = $2; appendLocInfo($$, @1); }
|   type "->" type                                              { if ((dynamic_cast<CustomType*>$3)->typeValue != TYPE_FUNC) $3 = new Function($3); appendLocInfo($3, @2); (dynamic_cast<Function*>$3)->params.push_back($1); $$ = $3; }
|   type "ref"                                                  { $$ = new Reference($1); appendLocInfo($$, @2); }
|   "array" "of" type                                           { $$ = new Array($3, 1); appendLocInfo($$, @1); }
|   "array" '[' comma_star_gen ']' "of" type                    { $$ = new Array($6, $3); appendLocInfo($$, @1); }
|   "id"                                                        { $$ = new CustomType($1); appendLocInfo($$, @1); }
;

comma_star_gen: 
    '*'                                                         { $$ = 1; }
|   '*' ',' comma_star_gen                                      { $$ = 1 + $3; }
;

expr_high:
    '!' expr_high                                               { $$ = new UnOp("!", $2); appendLocInfo($$, @1); }
|   '(' expr ')'                                                { st.openScope(); $$ = $2; st.closeScope(); currPseudoScope = currPseudoScope->getNext(); appendLocInfo($$, @1); currPseudoScope = currPseudoScope->getPrev(); }
|   "int_const"                                                 { $$ = new IntConst($1); appendLocInfo($$, @1); }
|   "float_const"                                               { $$ = new FloatConst($1); appendLocInfo($$, @1); }
|   "char_const"                                                { $$ = new CharConst($1); appendLocInfo($$, @1); }
|   "string_literal"                                            { $$ = new StringLiteral($1); appendLocInfo($$, @1); }
|   "true"                                                      { $$ = new BooleanConst(true); appendLocInfo($$, @1); }
|   "false"                                                     { $$ = new BooleanConst(false); appendLocInfo($$, @1); }
|   '(' ')'                                                     { $$ = new UnitConst(); appendLocInfo($$, @1); }
|   "id" '[' expr comma_expr_gen ']'                            { $$ = new ArrayItem($1, $3, $4); appendLocInfo($$, @1); }
|   "id"                                                        { $$ = new Id($1); appendLocInfo($$, @1); }
|   "Id"                                                        { $$ = new Constr($1, nullptr, nullptr); appendLocInfo($$, @1); }
;

expr:
    '+' expr %prec SIGN                                         { $$ = new UnOp("+", $2); appendLocInfo($$, @1); }
|   '-' expr %prec SIGN                                         { $$ = new UnOp("-", $2); appendLocInfo($$, @1); }
|   "+." expr %prec SIGN                                        { $$ = new UnOp("+.", $2); appendLocInfo($$, @1); }
|   "-." expr %prec SIGN                                        { $$ = new UnOp("-.", $2); appendLocInfo($$, @1); }
|   "not" expr                                                  { $$ = new UnOp("not", $2); appendLocInfo($$, @1); }
|   expr '+' expr                                               { $$ = new BinOp($1, "+", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr '-' expr                                               { $$ = new BinOp($1, "-", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr '*' expr                                               { $$ = new BinOp($1, "*", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr '/' expr                                               { $$ = new BinOp($1, "/", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr "+." expr                                              { $$ = new BinOp($1, "+.", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr "-." expr                                              { $$ = new BinOp($1, "-.", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr "*." expr                                              { $$ = new BinOp($1, "*.", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr "/." expr                                              { $$ = new BinOp($1, "/.", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr "mod" expr                                             { $$ = new BinOp($1, "mod", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr "**" expr                                              { $$ = new BinOp($1, "**", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr '=' expr                                               { $$ = new BinOp($1, "=", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr "<>" expr                                              { $$ = new BinOp($1, "<>", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr '<' expr                                               { $$ = new BinOp($1, "<", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr '>' expr                                               { $$ = new BinOp($1, ">", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr "<=" expr                                              { $$ = new BinOp($1, "<=", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr ">=" expr                                              { $$ = new BinOp($1, ">=", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr "==" expr                                              { $$ = new BinOp($1, "==", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr "!=" expr                                              { $$ = new BinOp($1, "!=", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr "&&" expr                                              { $$ = new BinOp($1, "&&", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr "||" expr                                              { $$ = new BinOp($1, "||", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr ';' expr                                               { $$ = new BinOp($1, ";", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   expr ":=" expr                                              { $$ = new BinOp($1, ":=", $3); appendLocInfo($$, @1); changeEnd($$, @3); }
|   "id" expr_high expr_high_gen                                { $$ = new Id($1, $2, $3); appendLocInfo($$, @1); }
|   "Id" expr_high expr_high_gen                                { $$ = new Constr($1, $2, $3); appendLocInfo($$, @1); }
|   "dim" "id"                                                  { $$ = new Dim($2); appendLocInfo($$, @2); }
|   "dim" "int_const" "id"                                      { $$ = new Dim($3, $2); appendLocInfo($$, @3); }
|   "new" type                                                  { $$ = new New($2); appendLocInfo($$, @1); }
|   "delete" expr                                               { $$ = new Delete($2); appendLocInfo($$, @1); }
|   letdef "in" expr %prec LETIN                                { $$ = new LetIn($1, $3); appendLocInfo($$, @2); } // block
|   "begin" expr "end" /* possible to go to expr_high */        { $$ = new Begin($2); appendLocInfo($$, @1); } // block
|   "if" expr "then" expr                                       { $$ = new If($2, $4, nullptr); appendLocInfo($$, @1); } // block
|   "if" expr "then" expr "else" expr                           { $$ = new If($2, $4, $6); appendLocInfo($$, @1); } // block
|   "while" expr "do" expr "done"                               { $$ = new While($2, $4); appendLocInfo($$, @1); } // block
|   "for" "id" '=' expr "to" expr "do" expr "done"              { $$ = new For($2, $4, $6, $8, true); appendLocInfo($$, @1); } // block
|   "for" "id" '=' expr "downto" expr "do" expr "done"          { $$ = new For($2, $4, $6, $8, false); appendLocInfo($$, @1); } // block
|   "match" expr "with" clause bar_clause_gen "end"             { $$ = new Match($2, $4, $5); appendLocInfo($$, @1); }
|   expr_high                                                   { $$ = $1; }
;

expr_high_gen: %empty
    /* nothing */                                               { $$ = nullptr; }
|   expr_high expr_high_gen                                     { $$ = new ExprGen($1, $2); appendLocInfo($$, @1); }
;

bar_clause_gen: %empty 
    /* nothing */                                               { $$ = nullptr; }
|   '|' clause bar_clause_gen                                   { $$ = new BarClauseGen($2, $3); appendLocInfo($$, @1); }
;

clause: 
    pattern "->" expr                                           { $$ = new Clause($1, $3); appendLocInfo($$, @2); }
;

pattern:
    pattern_high                                                { $$ = $1; }
|   "Id" pattern_high_gen                                       { $$ = new PatternConstr($1, $2); appendLocInfo($$, @1); }
;

pattern_high:
    '+' "int_const" %prec SIGN                                  { $$ = new IntConst($2, '+', true); appendLocInfo($$, @1); }
|   '-' "int_const" %prec SIGN                                  { $$ = new IntConst($2, '-', true); appendLocInfo($$, @1); }
|   "int_const" %prec SIGN                                      { $$ = new IntConst($1, true); appendLocInfo($$, @1); }
|   "+." "float_const" %prec SIGN                               { $$ = new FloatConst($2, "+.", true); appendLocInfo($$, @1); }
|   "-." "float_const" %prec SIGN                               { $$ = new FloatConst($2, "-.", true); appendLocInfo($$, @1); }
|   "float_const" %prec SIGN                                    { $$ = new FloatConst($1, true); appendLocInfo($$, @1); }
|   "char_const"                                                { $$ = new CharConst($1, true); appendLocInfo($$, @1); }
|   "true"                                                      { $$ = new BooleanConst(true, true); appendLocInfo($$, @1); }
|   "false"                                                     { $$ = new BooleanConst(false, true); appendLocInfo($$, @1); }
|   "id"                                                        { $$ = new PatternId($1); appendLocInfo($$, @1); }
|   '(' pattern ')'                                             { $$ = $2; }
;

pattern_high_gen: %empty
    /* nothing */                                               { $$ = nullptr; }
|   pattern_high pattern_high_gen                               { $$ = new PatternGen($1, $2); appendLocInfo($$, @1); }
;

%%

int main(int argc, char **argv) {
    yydebug = 0;

    optionsMenu->init();
    optionsMenu->parse(argc, argv);
    
    int result = yyparse();

    optionsMenu->execute();
    // if (result == 0) printf("Success.\n");
    return result;
}
