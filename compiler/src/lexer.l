%{

#define T_eof 0
#define T_and 1000
#define T_array 1001
#define T_begin 1002
#define T_bool 1003
#define T_char 1004
#define T_delete 1005
#define T_dim 1006
#define T_do 1007
#define T_done 1008
#define T_downto 1009
#define T_else 1010
#define T_end 1011
#define T_false 1012
#define T_float 1013
#define T_for 1014
#define T_if 1015
#define T_in 1016
#define T_int 1017
#define T_let 1018
#define T_match 1019
#define T_mod 1020
#define T_mutable 1021
#define T_new 1022
#define T_not 1023
#define T_of 1024
#define T_rec 1025
#define T_ref 1026
#define T_then 1027
#define T_to 1028
#define T_true 1029
#define T_type 1030
#define T_unit 1031
#define T_while 1032
#define T_with 1033
#define T_op_arrow 1034
#define T_op_fadd 1035
#define T_op_fsub 1036
#define T_op_fmul 1037
#define T_op_fdiv 1038
#define T_op_fpow 1039
#define T_op_and 1040
#define T_op_or 1041
#define T_op_struct_neq 1042
#define T_op_leq 1043
#define T_op_geq 1044
#define T_op_eq 1045
#define T_op_phys_neq 1046
#define T_op_assign 1047

#define T_id 1048
#define T_constructor 1049
#define T_const 1050
#define T_const_float 1051
#define T_const_string 1052
#define T_const_char 1053

int lineno = 1;
int comment_depth = 0;

%}

L [a-zA-Z]
H [0-9a-fA-F]
D [0-9]
W ([\n\t\r\0\\\'\"]|\x{H}{H})


%option noyywrap

%x COMMENT

%%

"and"     { return T_and; }
"array"   { return T_array; }
"begin"   { return T_begin; }
"bool"    { return T_bool; }
"char"    { return T_char; }
"delete"  { return T_delete; }
"dim"     { return T_dim; }
"do"      { return T_do; }
"done"    { return T_done; }
"downto"  { return T_downto; }
"else"    { return T_else; }
"end"     { return T_end; }
"false"   { return T_false; }
"float"   { return T_float; }
"for"     { return T_for; }
"if"      { return T_if; }
"in"      { return T_in; }
"int"     { return T_int; }
"let"     { return T_let; }
"match"   { return T_match; }
"mod"     { return T_mod; }
"mutable" { return T_mutable; }
"new"     { return T_new; }
"not"     { return T_not; }
"of"      { return T_of; }
"rec"     { return T_rec; }
"ref"     { return T_ref; }
"then"    { return T_then; }
"to"      { return T_to; }
"true"    { return T_true; }
"type"    { return T_type; }
"unit"    { return T_unit; }
"while"   { return T_while; }
"with"    { return T_with; }


"->"      { return T_op_arrow; }
"+."      { return T_op_fadd; }
"-."      { return T_op_fsub; }
"*."      { return T_op_fmul; }
"/."      { return T_op_fdiv; }
"**"      { return T_op_fpow; }
"&&"      { return T_op_and; }
"||"      { return T_op_or; }
"<>"      { return T_op_struct_neq; }
"<="      { return T_op_leq; }
">="      { return T_op_geq; }
"=="      { return T_op_eq; }
"!="      { return T_op_phys_neq; }
":="      { return T_op_assign; }

[\=\|\+\-\*\/\!\;\<\>\(\)\[\]\,\:]  { return yytext[0]; }

{D}+                                { return T_const; }
{D}+\.{D}+([Ee][\+\-]?{D}+)?        { return T_const_float; }

\'{W}\'                             { return T_const_char; }
\"(\\.|[^"])*\"                     { return T_const_string; }
 
[a-z]({L}|{D}|_)*                   { return T_id; }
[A-Z]({L}|{D}|_)*                   { return T_constructor; }


"--".*\n                            { lineno++; }

"(*"                                { BEGIN(COMMENT); comment_depth++; }
<COMMENT>"(*"                       { comment_depth++; }
<COMMENT>"*)"                       { if (!--comment_level) BEGIN(INITIAL); }
<COMMENT>\n                         { lineno++; }
<COMMENT>[(|*]                      { /* do nothing */ }
<COMMENT>[^(*\n]+                   { /* do nothing */ }
<COMMENT><<EOF>>                    { fprintf(stderr, "Unfinished comment at EOF\n"); exit(1); }

\n                                  { lineno++; }
[ \t\r]                             { /* do nothing */ }

.                                   { fprintf(stderr, "Illegal character '%c'\n", yytext[0]); exit(1); }

%%

int main(){
   int token;
   do {
      token = yylex();
      printf("token=%d, lexeme='%s'\n", token, yytext);
   } while (token != T_eof);
}
