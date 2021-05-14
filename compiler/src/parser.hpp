/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_PARSER_HPP_INCLUDED
# define YY_YY_PARSER_HPP_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    T_and = 258,
    T_array = 259,
    T_begin = 260,
    T_bool = 261,
    T_char = 262,
    T_delete = 263,
    T_dim = 264,
    T_do = 265,
    T_done = 266,
    T_downto = 267,
    T_else = 268,
    T_end = 269,
    T_false = 270,
    T_float = 271,
    T_for = 272,
    T_if = 273,
    T_in = 274,
    T_int = 275,
    T_let = 276,
    T_match = 277,
    T_mod = 278,
    T_mutable = 279,
    T_new = 280,
    T_not = 281,
    T_of = 282,
    T_rec = 283,
    T_ref = 284,
    T_then = 285,
    T_to = 286,
    T_true = 287,
    T_type = 288,
    T_unit = 289,
    T_while = 290,
    T_with = 291,
    T_op_arrow = 292,
    T_op_fadd = 293,
    T_op_fsub = 294,
    T_op_fmul = 295,
    T_op_fdiv = 296,
    T_op_fpow = 297,
    T_op_and = 298,
    T_op_or = 299,
    T_op_struct_neq = 300,
    T_op_leq = 301,
    T_op_geq = 302,
    T_op_eq = 303,
    T_op_phys_neq = 304,
    T_op_assign = 305,
    T_id = 306,
    T_constructor = 307,
    T_const = 308,
    T_const_float = 309,
    T_const_string = 310,
    T_const_char = 311,
    LETIN = 312,
    SIGN = 313
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 92 "parser.y"

    AST *ast;
    int num;
    float flt;
    char chr;
    int comma_star_gen;
AST *program;
AST *stmt_list;
Let *letdef;
DefGen *def_gen;
Def *def;
ParGen *par_gen;
CommaExprGen *comma_expr_gen;
AST *type_def;
TdefGen *tdef_gen;
Tdef *tdef;
BarConstrGen *bar_constr_gen;
Constr *constr;
TypeGen *type_gen;
Par *par;
Type *type;
//*comma_star_gen;
Expr *expr_high;
Expr *expr;
ExprGen *expr_high_gen;
BarClauseGen *bar_clause_gen;
Clause *clause;
Pattern *pattern;
Pattern *pattern_high;
PatternGen *pattern_high_gen;

#line 148 "parser.hpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_HPP_INCLUDED  */
