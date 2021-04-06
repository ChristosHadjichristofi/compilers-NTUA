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

%token<str> T_id            "id"
%token<str> T_constructor   "Id"
%token<num> T_const         "int_const"
%token<flt> T_const_float   "float_const"
%token<str> T_const_string  "string_literal"
%token<chr> T_const_char    "char_const"

/* Priority and associativity of Llama operators. */
/* each line in this list is a level of priority
   each line in this list has higher priority 
   than the line before
*/
%nonassoc "let" "in"
%left ';'
%nonassoc "if" "then"
%nonassoc "else"
%nonassoc ":="
%left "||"
%left "&&"
%nonassoc '=' "<>" '>' '<' "<=" ">=" "==" "!="
%left '+' '-' "+." "-."
%left '*' '/' "*." "/." "mod"
%right "**"
%nonassoc "not" "delete"
%nonassoc '!' 
%nonassoc "new"

%union {
    string str;
    int num;
    float flt;
    char chr;
}



%%

program: stmt_list;

stmt_list: %empty  
    /* nothing */
|   letdef stmt_list
|   typedef stmt_list
;

letdef:
    "let" def def_gen
|   "let" "rec" def def_gen
;

def_gen: %empty  
    /* nothing */
|   "and" def def_gen
;

def: 
    "id" par_gen '=' expr
|   "id" par_gen ':' type '=' expr
|   "mutable" "id"
|   "mutable" "id" ':' type
|   "mutable" "id" '[' expr comma_expr_gen ']'
|   "mutable" "id" '[' expr comma_expr_gen ']' ':' type
;

par_gen:
    par
|   par par_gen;

comma_expr_gen: %empty 
    /* nothing */
|   "," expr comma_expr_gen
;

typedef: "type" tdef_gen;

tdef_gen: 
    tdef
|   tdef "and" tdef_gen
;

tdef: "id" "=" constr bar_constr_gen;

bar_constr_gen: %empty 
    /* nothing */
|   "|" constr bar_constr_gen
;

constr: "Id" "of" type_gen;

type_gen: 
    type 
|   type type_gen;

par:
    "id"
|   "(" "id" ":" type ")"
;

type: 
    "unit" 
|   "int" 
|   "char" 
|   "bool" 
|   "float"
|   "(" type ")"
|   type "->" type
|   type "ref"
|   "array" "of" type
|   "array" "[" comma_star_gen "]" "of" type
|   "id"
;

comma_star_gen: 
    "*"
|   "*" "," comma_star_gen
;

expr_high:
    "!" expr_high
|   "(" expr ")" 
|   "int_const" 
|   "float_const" 
|   "char_const" 
|   "string_literal" 
|   "true"    
|   "false"   
|   "(" ")" 
|   "id" "[" expr comma_expr_gen "]" 
|   "id"
|   "Id"
;



expr:
    "+" expr
|   "-" expr
|   "+." expr
|   "-." expr
|   "not" expr
|   expr '+' expr
|   expr '-' expr
|   expr '*' expr
|   expr '/' expr
|   expr "+." expr
|   expr "-." expr
|   expr "*." expr
|   expr "/." expr
|   expr "mod" expr
|   expr "**" expr
|   expr '=' expr
|   expr "<>" expr
|   expr '<' expr
|   expr '>' expr
|   expr "<=" expr
|   expr ">=" expr
|   expr "==" expr
|   expr "!=" expr
|   expr "&&" expr
|   expr "||" expr
|   expr ';' expr
|   expr ":=" expr
|   "id" expr_high_gen
|   "Id" expr_high_gen
|   "dim" "id"
|   "dim" "int_const" "id"
|   "new" type
|   "delete" expr
|   letdef "in" expr
|   "begin" expr "end"
|   "if" expr "then" expr
|   "if" expr "then" expr "else" expr
|   "while" expr "do" expr "done"
|   "for" "id" "=" expr "to" expr "do" expr "done"
|   "for" "id" "=" expr "downto" expr "do" expr "done"
|   "match" expr "with" clause bar_clause_gen "end"
;

expr_high_gen:
    expr_high
|   expr_high expr_high_gen
;

bar_clause_gen: %empty 
    /* nothing */
|   "|" clause bar_clause_gen
;

clause: pattern "->" expr;

pattern:
    pattern_high
|   "Id" pattern_high_gen
;

pattern_high:
    "+" "int_const"
|   "-" "int_const"
|   "+." "float_const"
|   "-." "float_const"
|   "char_const"
|   "true"
|   "false"
|   "id"
|   "(" pattern ")"
;

pattern_high_gen: %empty
    /* nothing */
|   pattern_high pattern_high_gen
;

%%