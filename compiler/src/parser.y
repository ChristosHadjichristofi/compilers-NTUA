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

// %token T_id
// %token T_constructor
// %token T_const
// %token T_const_float
// %token T_const_string
// %token T_const_char 

/* Priority and associativity of Llama operators. */
/* each line in this list is a level of priority
   each line in this list has higher priority 
   than the line before
*/
%nonassoc "let" "in"
%left ";"
%nonassoc "if" "then"
%nonassoc "else"
%nonassoc ":="
%left "||"
%left "&&"
%nonassoc "=" "<>" ">" "<" "<=" ">=" "==" "!="
%left "+" "-" "+." "-."
%left "*" "/" "*." "/." "mod"
%right "**"
%nonassoc "+" "-" "+." "-." "not" "delete"
%nonassoc "!" 
%nonassoc "new"

%%

program: stmt_list;

stmt_list: 
    /* nothing */
|   letdef stmt_list
|   typedef stmt_list
;

letdef:
    "let" def def_gen
|   "let" "rec" def def_gen
;

def_gen: 
    /* nothing */
|   "and" def def_gen
;

def: 
    id par type_optional "=" expr
|   "mutable" id type_optional
|   "mutable" id "[" expr comma_expr_gen "]" type_optional
;

comma_expr_gen:
    /* nothing */
|   "," expr comma_expr_gen
;

type_optional:
    /* nothing */
|   ":" type
;

typedef: "type" tdef tdef_gen;

tdef_gen:
    /* nothing */
|   "and" tdef tdef_gen
;

tdef: id "=" constr bar_constr_gen;

bar_constr_gen:
    /* nothing */
|   "|" constr bar_constr_gen
;

constr: Id "of" type_gen;

type_gen: 
    type 
|   type type_gen;

par:
    id
|   "(" id ":" type ")"
;

type: 
    "unit" | "int" | "char" | "bool" | "float"
|   "(" type ")"
|   type "->" type
|   type "ref"
|   "array" "of" type
|   "array" "[" "*" comma_star_gen "]" "of" type
|   id
;

comma_star_gen:
    /* nothing */
|   "," "*" comma_star_gen
;

expr:
    int_const | float_const | char_const | string_literal 
|   "true"    |   "false"   | "(" ")" 
|   "(" expr ")" 
|   unop expr 
|   expr binop expr
|   id expr_gen
|   Id expr_gen
|   id "[" expr comma_expr_gen "]" 
|   "dim" id
|   "dim" int_const id
|   "new" type
|   "delete" expr
|   letdef "in" expr
|   "begin" expr "end"
|   "if" expr "then" expr
|   "if" expr "then" expr "else" expr
|   "while" expr "do" expr "done"
|   "for" id "=" expr "to" expr "do" expr "done"
|   "for" id "=" expr "downto" expr "do" expr "done"
|   "match" expr "with" clause bar_clause_gen "end"
;

expr_gen:
    /* nothing */
|   expr expr_gen
;

bar_clause_gen:
    /* nothing */
|   "|" clause bar_clause_gen
;

unop: "+" | "-" | "+." | "-." | "!" | "not";

binop: 
    "+" | "-"  | "*"  | "/" | "+." | "-." | "*." | "/." | "mod"
|  "**" | "="  | "<>" | "<" | ">"  | "<=" | ">=" | "==" | "!=" 
|  "&&" | "||" | ";"  | ":=";

clause: pattern "->" expr;

pattern:
    "+" int_const
|   "-" int_const
|   "+." float_const
|   "-." float_const
|   char_const
|   "true"
|   "false"
|   id
|   "(" pattern ")"
|   Id pattern_gen
;

pattern_gen:
    /* nothing */
|   pattern pattern_gen
;

%%