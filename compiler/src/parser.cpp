/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parser.y"

#define YYDEBUG 1
#include <cstdio>
#include <string>
#include "lexer.hpp"
#include "ast.hpp"

#line 78 "parser.cpp"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
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

#line 221 "parser.cpp"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_PARSER_HPP_INCLUDED  */



#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))

/* Stored state numbers (used for stacks). */
typedef yytype_uint8 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  13
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   987

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  75
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  25
/* YYNRULES -- Number of rules.  */
#define YYNRULES  116
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  239

#define YYUNDEFTOK  2
#define YYMAXUTOK   313


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    74,     2,     2,     2,     2,     2,     2,
      72,    73,    64,    62,    70,    63,     2,    65,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    67,    58,
      61,    59,    60,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    68,     2,    69,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    71,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    66
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   155,   155,   160,   162,   163,   167,   168,   171,   173,
     177,   178,   179,   180,   181,   182,   185,   187,   190,   192,
     196,   199,   201,   205,   208,   210,   214,   215,   219,   220,
     224,   225,   229,   230,   231,   232,   233,   234,   235,   236,
     237,   238,   239,   243,   244,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,   293,   294,   295,
     296,   297,   298,   299,   300,   301,   302,   303,   304,   307,
     309,   312,   314,   318,   322,   323,   327,   328,   329,   330,
     331,   332,   333,   334,   335,   338,   340
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "\"and\"", "\"array\"", "\"begin\"",
  "\"bool\"", "\"char\"", "\"delete\"", "\"dim\"", "\"do\"", "\"done\"",
  "\"downto\"", "\"else\"", "\"end\"", "\"false\"", "\"float\"", "\"for\"",
  "\"if\"", "\"in\"", "\"int\"", "\"let\"", "\"match\"", "\"mod\"",
  "\"mutable\"", "\"new\"", "\"not\"", "\"of\"", "\"rec\"", "\"ref\"",
  "\"then\"", "\"to\"", "\"true\"", "\"type\"", "\"unit\"", "\"while\"",
  "\"with\"", "\"->\"", "\"+.\"", "\"-.\"", "\"*.\"", "\"/.\"", "\"**\"",
  "\"&&\"", "\"||\"", "\"<>\"", "\"<=\"", "\">=\"", "\"==\"", "\"!=\"",
  "\":=\"", "\"id\"", "\"Id\"", "\"int_const\"", "\"float_const\"",
  "\"string_literal\"", "\"char_const\"", "LETIN", "';'", "'='", "'>'",
  "'<'", "'+'", "'-'", "'*'", "'/'", "SIGN", "':'", "'['", "']'", "','",
  "'|'", "'('", "')'", "'!'", "$accept", "program", "stmt_list", "letdef",
  "def_gen", "def", "par_gen", "comma_expr_gen", "type_def", "tdef_gen",
  "tdef", "bar_constr_gen", "constr", "type_gen", "par", "type",
  "comma_star_gen", "expr_high", "expr", "expr_high_gen", "bar_clause_gen",
  "clause", "pattern", "pattern_high", "pattern_high_gen", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,    59,    61,
      62,    60,    43,    45,    42,    47,   313,    58,    91,    93,
      44,   124,    40,    41,    33
};
# endif

#define YYPACT_NINF (-179)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      37,    77,   -36,    38,  -179,    37,    37,     6,    78,   -47,
      70,     5,   104,  -179,  -179,  -179,   -59,    70,  -179,    62,
       9,   -47,    78,  -179,    79,   -36,  -179,   202,   178,  -179,
      63,   178,   202,  -179,    70,   108,    66,   104,   -15,  -179,
    -179,  -179,  -179,  -179,  -179,   202,   105,   178,   178,   -48,
    -179,    81,   178,   178,   202,   178,  -179,   178,   178,   178,
      -9,    93,  -179,  -179,  -179,  -179,   178,   178,   101,    93,
     122,  -179,   681,   202,   782,     3,  -179,   202,    79,  -179,
    -179,   202,    86,   -19,  -179,   202,   610,  -179,  -179,    92,
      99,   724,   754,   105,  -179,   266,  -179,  -179,    83,  -179,
     178,    93,    93,  -179,  -179,  -179,   638,  -179,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,    90,   -18,   178,  -179,   208,    66,   131,    91,
      97,  -179,   105,  -179,  -179,   178,   178,   231,   178,   681,
      93,  -179,  -179,  -179,   782,   120,    10,    10,   120,   120,
     120,   894,   866,   922,   922,   922,   922,   922,   810,   838,
     922,   922,   922,    10,    10,   120,   120,   681,   102,  -179,
     782,  -179,  -179,    86,   141,   524,   567,  -179,  -179,   117,
     118,  -179,   228,  -179,   127,   129,   231,   113,   133,  -179,
     395,   121,  -179,  -179,   202,  -179,   202,   178,   178,   178,
    -179,  -179,   228,  -179,  -179,  -179,   112,   231,   175,   178,
    -179,  -179,   105,   131,   309,   352,   838,  -179,  -179,   113,
    -179,   782,   178,   178,  -179,   438,   481,  -179,  -179
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       3,     0,     0,     0,     2,     3,     3,     0,     0,    16,
       8,     0,    21,     1,     4,     5,    12,     8,    30,     0,
       0,    16,     0,     6,     0,     0,    20,     0,     0,     7,
       0,     0,     0,    17,     8,    26,    24,    21,     0,    35,
      34,    36,    33,    32,    42,     0,    13,     0,     0,     0,
      52,     0,     0,     0,     0,     0,    51,     0,     0,     0,
      55,    56,    47,    48,    50,    49,     0,     0,     0,     0,
       0,    98,    18,     0,    10,     0,     9,     0,     0,    23,
      22,     0,     0,     0,    39,     0,     0,    89,    86,     0,
       0,     0,     0,    88,    61,     0,    59,    60,    55,    56,
       0,    99,    99,    57,    58,    53,     0,    45,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    27,    28,    24,    40,    43,
       0,    37,    38,    91,    87,     0,     0,     0,     0,    18,
      99,    84,    85,    46,    90,    70,    66,    67,    68,    69,
      71,    80,    81,    73,    76,    77,    78,    79,    83,    82,
      72,    75,    74,    62,    63,    64,    65,    18,    14,    31,
      11,    29,    25,     0,     0,     0,    92,   112,   111,     0,
       0,   113,   115,   110,     0,     0,     0,   101,     0,   104,
       0,     0,   100,    19,     0,    44,     0,     0,     0,     0,
     108,   109,   115,   105,   106,   107,     0,     0,     0,     0,
      94,    54,    15,    41,     0,     0,    93,   116,   114,   101,
      97,   103,     0,     0,   102,     0,     0,    96,    95
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -179,  -179,   106,    61,    87,   116,   170,  -136,  -179,   155,
     169,    60,   142,    69,  -179,   -25,    15,   -30,   -31,  -101,
     -22,    -6,    23,  -178,    11
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     3,     4,    70,    23,    10,    20,   132,     6,    26,
      12,    79,    36,   135,    21,   136,   140,    71,    72,   151,
     218,   197,   198,   199,   213
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      74,   152,    46,    88,    18,    89,    50,    75,    27,    28,
      84,    84,    81,   201,   212,    11,    86,    87,    85,    85,
      83,    91,    92,    56,    94,    19,    95,    96,    97,    93,
     101,   102,    84,   109,   212,   103,   104,   106,    13,   107,
      85,   203,    98,    99,    62,    63,    64,    65,   133,   202,
     112,   113,   114,    82,   141,   179,   138,    16,     1,   100,
     142,     5,   134,    68,    24,    69,     5,     5,    31,   149,
       2,   150,   150,    22,   129,   130,    32,   154,   155,   156,
     157,   158,   159,   160,   161,   162,   163,   164,   165,   166,
     167,   168,   169,   170,   171,   172,   173,   174,   175,   176,
     177,     7,     7,   180,    29,     8,    47,    25,    50,    48,
      49,    14,    15,    30,   185,   186,    50,   200,    51,    52,
     150,    76,     1,    53,    17,    56,    54,    55,     9,     9,
      73,    35,    90,    56,    84,    77,    57,    78,    34,    58,
      59,   108,    85,   144,    98,    99,    62,    63,    64,    65,
     139,   100,    60,    61,    62,    63,    64,    65,   145,   178,
      84,   183,   114,    66,    67,    68,   184,    69,   206,   204,
     219,   210,   211,    68,   105,    69,   224,   225,   226,   222,
     214,   223,   215,    47,   217,   228,    48,    49,   231,   230,
     221,    33,    80,    50,    37,    51,    52,   182,   205,     1,
      53,   235,   236,    54,    55,   181,    38,   234,    39,    40,
      56,   229,    38,    57,    39,    40,    58,    59,    41,   216,
     137,     0,    42,   227,    41,     0,     0,     0,    42,    60,
      61,    62,    63,    64,    65,     0,    43,    84,     0,     0,
      66,    67,    43,   187,     0,    85,   187,     0,     0,     0,
      68,     0,    69,    44,     0,     0,     0,     0,     0,    44,
     188,     0,     0,   188,     0,     0,   189,   190,     0,   189,
     190,     0,     0,     0,    45,     0,   148,     0,     0,   191,
      45,     0,   191,   192,   193,     0,     0,   193,     0,   109,
     194,   195,     0,   194,   195,     0,     0,     0,     0,     0,
     196,     0,     0,   196,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,   120,   121,   122,     0,     0,   232,
       0,     0,     0,     0,   123,   124,   125,   126,   127,   128,
     129,   130,   109,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,   120,   121,   122,
       0,     0,   233,     0,     0,     0,     0,   123,   124,   125,
     126,   127,   128,   129,   130,   109,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,     0,     0,     0,   220,     0,     0,     0,
     123,   124,   125,   126,   127,   128,   129,   130,   109,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,   120,   121,   122,     0,     0,     0,   237,
       0,     0,     0,   123,   124,   125,   126,   127,   128,   129,
     130,   109,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,     0,
       0,     0,   238,     0,     0,     0,   123,   124,   125,   126,
     127,   128,   129,   130,   109,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,     0,     0,     0,     0,   207,     0,     0,   123,
     124,   125,   126,   127,   128,   129,   130,   109,     0,     0,
       0,     0,     0,     0,     0,   208,     0,     0,     0,     0,
       0,     0,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,     0,     0,     0,     0,     0,
     209,     0,   123,   124,   125,   126,   127,   128,   129,   130,
     109,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,     0,     0,
       0,     0,     0,     0,   143,     0,   124,   125,   126,   127,
     128,   129,   130,   109,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
     122,   109,     0,     0,     0,     0,     0,     0,   123,   124,
     125,   126,   127,   128,   129,   130,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,     0,
       0,     0,     0,     0,     0,     0,   123,   124,   125,   126,
     127,   128,   129,   130,   109,     0,     0,     0,     0,     0,
       0,   153,     0,     0,     0,     0,     0,     0,     0,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,     0,     0,     0,     0,     0,     0,     0,   123,
     124,   125,   126,   127,   128,   129,   130,   109,     0,     0,
       0,   131,     0,     0,   146,     0,     0,     0,     0,     0,
       0,     0,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,     0,     0,   109,     0,     0,
       0,     0,   123,   124,   125,   126,   127,   128,   129,   130,
     147,     0,   110,   111,   112,   113,   114,   115,   116,   117,
     118,   119,   120,   121,   122,   109,     0,     0,     0,     0,
       0,     0,   123,   124,   125,   126,   127,   128,   129,   130,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,   122,   109,     0,     0,     0,     0,     0,     0,
     123,   124,   125,   126,   127,   128,   129,   130,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,   120,   121,
      -1,   109,     0,     0,     0,     0,     0,     0,     0,   124,
     125,   126,   127,   128,   129,   130,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,   121,   122,   109,
       0,     0,     0,     0,     0,     0,     0,   124,   125,   126,
     127,   128,   129,   130,   110,   111,   112,   113,   114,   115,
       0,   117,   118,   119,   120,   121,     0,   109,     0,     0,
       0,     0,     0,     0,     0,   124,   125,   126,   127,   128,
     129,   130,   110,   111,   112,   113,   114,     0,     0,   117,
     118,   119,   120,   121,     0,   109,     0,     0,     0,     0,
       0,     0,     0,   124,   125,   126,   127,   128,   129,   130,
     110,   111,   112,   113,   114,     0,     0,    -1,    -1,    -1,
      -1,    -1,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    -1,    -1,    -1,   127,   128,   129,   130
};

static const yytype_int16 yycheck[] =
{
      31,   102,    27,    51,    51,    53,    15,    32,    67,    68,
      29,    29,    27,   149,   192,    51,    47,    48,    37,    37,
      45,    52,    53,    32,    55,    72,    57,    58,    59,    54,
      60,    61,    29,    23,   212,    66,    67,    68,     0,    69,
      37,   177,    51,    52,    53,    54,    55,    56,    73,   150,
      40,    41,    42,    68,    73,    73,    81,    51,    21,    68,
      85,     0,    59,    72,    59,    74,     5,     6,    59,   100,
      33,   101,   102,     3,    64,    65,    67,   108,   109,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,   120,
     121,   122,   123,   124,   125,   126,   127,   128,   129,   130,
     131,    24,    24,   134,    17,    28,     5,     3,    15,     8,
       9,     5,     6,    51,   145,   146,    15,   148,    17,    18,
     150,    34,    21,    22,     8,    32,    25,    26,    51,    51,
      67,    52,    51,    32,    29,    27,    35,    71,    22,    38,
      39,    19,    37,    51,    51,    52,    53,    54,    55,    56,
      64,    68,    51,    52,    53,    54,    55,    56,    59,    69,
      29,    70,    42,    62,    63,    72,    69,    74,    27,    67,
      37,    54,    54,    72,    73,    74,   207,   208,   209,   204,
      53,   206,    53,     5,    71,    73,     8,     9,   219,    14,
      69,    21,    37,    15,    25,    17,    18,   137,   183,    21,
      22,   232,   233,    25,    26,   136,     4,   229,     6,     7,
      32,   217,     4,    35,     6,     7,    38,    39,    16,   196,
      78,    -1,    20,   212,    16,    -1,    -1,    -1,    20,    51,
      52,    53,    54,    55,    56,    -1,    34,    29,    -1,    -1,
      62,    63,    34,    15,    -1,    37,    15,    -1,    -1,    -1,
      72,    -1,    74,    51,    -1,    -1,    -1,    -1,    -1,    51,
      32,    -1,    -1,    32,    -1,    -1,    38,    39,    -1,    38,
      39,    -1,    -1,    -1,    72,    -1,    10,    -1,    -1,    51,
      72,    -1,    51,    52,    56,    -1,    -1,    56,    -1,    23,
      62,    63,    -1,    62,    63,    -1,    -1,    -1,    -1,    -1,
      72,    -1,    -1,    72,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    -1,    -1,    10,
      -1,    -1,    -1,    -1,    58,    59,    60,    61,    62,    63,
      64,    65,    23,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      -1,    -1,    10,    -1,    -1,    -1,    -1,    58,    59,    60,
      61,    62,    63,    64,    65,    23,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    -1,    -1,    -1,    11,    -1,    -1,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    23,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    -1,    -1,    -1,    11,
      -1,    -1,    -1,    58,    59,    60,    61,    62,    63,    64,
      65,    23,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    -1,
      -1,    -1,    11,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    23,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    12,    -1,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    23,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    -1,    -1,    -1,    -1,    -1,
      13,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      23,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    14,    -1,    59,    60,    61,    62,
      63,    64,    65,    23,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    23,    -1,    -1,    -1,    -1,    -1,    -1,    58,    59,
      60,    61,    62,    63,    64,    65,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    58,    59,    60,    61,
      62,    63,    64,    65,    23,    -1,    -1,    -1,    -1,    -1,
      -1,    73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    58,
      59,    60,    61,    62,    63,    64,    65,    23,    -1,    -1,
      -1,    70,    -1,    -1,    30,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    -1,    -1,    23,    -1,    -1,
      -1,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      36,    -1,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    23,    -1,    -1,    -1,    -1,
      -1,    -1,    58,    59,    60,    61,    62,    63,    64,    65,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    23,    -1,    -1,    -1,    -1,    -1,    -1,
      58,    59,    60,    61,    62,    63,    64,    65,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    23,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,
      60,    61,    62,    63,    64,    65,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    23,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    59,    60,    61,
      62,    63,    64,    65,    38,    39,    40,    41,    42,    43,
      -1,    45,    46,    47,    48,    49,    -1,    23,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    59,    60,    61,    62,    63,
      64,    65,    38,    39,    40,    41,    42,    -1,    -1,    45,
      46,    47,    48,    49,    -1,    23,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    59,    60,    61,    62,    63,    64,    65,
      38,    39,    40,    41,    42,    -1,    -1,    45,    46,    47,
      48,    49,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    59,    60,    61,    62,    63,    64,    65
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    21,    33,    76,    77,    78,    83,    24,    28,    51,
      80,    51,    85,     0,    77,    77,    51,    80,    51,    72,
      81,    89,     3,    79,    59,     3,    84,    67,    68,    79,
      51,    59,    67,    81,    80,    52,    87,    85,     4,     6,
       7,    16,    20,    34,    51,    72,    90,     5,     8,     9,
      15,    17,    18,    22,    25,    26,    32,    35,    38,    39,
      51,    52,    53,    54,    55,    56,    62,    63,    72,    74,
      78,    92,    93,    67,    93,    90,    79,    27,    71,    86,
      84,    27,    68,    90,    29,    37,    93,    93,    51,    53,
      51,    93,    93,    90,    93,    93,    93,    93,    51,    52,
      68,    92,    92,    93,    93,    73,    93,    92,    19,    23,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    58,    59,    60,    61,    62,    63,    64,
      65,    70,    82,    90,    59,    88,    90,    87,    90,    64,
      91,    73,    90,    14,    51,    59,    30,    36,    10,    93,
      92,    94,    94,    73,    93,    93,    93,    93,    93,    93,
      93,    93,    93,    93,    93,    93,    93,    93,    93,    93,
      93,    93,    93,    93,    93,    93,    93,    93,    69,    73,
      93,    88,    86,    70,    69,    93,    93,    15,    32,    38,
      39,    51,    52,    56,    62,    63,    72,    96,    97,    98,
      93,    82,    94,    82,    67,    91,    27,    12,    31,    13,
      54,    54,    98,    99,    53,    53,    97,    71,    95,    37,
      11,    69,    90,    90,    93,    93,    93,    99,    73,    96,
      14,    93,    10,    10,    95,    93,    93,    11,    11
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_int8 yyr1[] =
{
       0,    75,    76,    77,    77,    77,    78,    78,    79,    79,
      80,    80,    80,    80,    80,    80,    81,    81,    82,    82,
      83,    84,    84,    85,    86,    86,    87,    87,    88,    88,
      89,    89,    90,    90,    90,    90,    90,    90,    90,    90,
      90,    90,    90,    91,    91,    92,    92,    92,    92,    92,
      92,    92,    92,    92,    92,    92,    92,    93,    93,    93,
      93,    93,    93,    93,    93,    93,    93,    93,    93,    93,
      93,    93,    93,    93,    93,    93,    93,    93,    93,    93,
      93,    93,    93,    93,    93,    93,    93,    93,    93,    93,
      93,    93,    93,    93,    93,    93,    93,    93,    93,    94,
      94,    95,    95,    96,    97,    97,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    99,    99
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     2,     2,     3,     4,     0,     3,
       4,     6,     2,     4,     6,     8,     0,     2,     0,     3,
       3,     0,     3,     4,     0,     3,     1,     3,     1,     2,
       1,     5,     1,     1,     1,     1,     1,     3,     3,     2,
       3,     6,     1,     1,     3,     2,     3,     1,     1,     1,
       1,     1,     1,     2,     5,     1,     1,     2,     2,     2,
       2,     2,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     2,     3,     2,     2,
       3,     3,     4,     6,     5,     9,     9,     6,     1,     0,
       2,     0,     3,     3,     1,     2,     2,     2,     2,     2,
       1,     1,     1,     1,     3,     0,     2
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[+yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[+*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2:
#line 155 "parser.y"
              {
        std::cout << "AST: " << *(yyvsp[0].stmt_list) << std::endl;
    }
#line 1714 "parser.cpp"
    break;

  case 3:
#line 161 "parser.y"
                                                                { (yyval.stmt_list) = nullptr; }
#line 1720 "parser.cpp"
    break;

  case 4:
#line 162 "parser.y"
                                                                { (yyvsp[-1].letdef)->merge((yyvsp[0].stmt_list)); (yyval.stmt_list) = (yyvsp[-1].letdef); }
#line 1726 "parser.cpp"
    break;

  case 5:
#line 163 "parser.y"
                                                                 { (yyvsp[-1].type_def)->merge((yyvsp[0].stmt_list)); (yyval.stmt_list) = (yyvsp[-1].type_def); }
#line 1732 "parser.cpp"
    break;

  case 6:
#line 167 "parser.y"
                                                                { (yyval.letdef) = new Let((yyvsp[-1].def), (yyvsp[0].def_gen), false); }
#line 1738 "parser.cpp"
    break;

  case 7:
#line 168 "parser.y"
                                                                { (yyval.letdef) = new Let((yyvsp[-1].def), (yyvsp[0].def_gen), true); }
#line 1744 "parser.cpp"
    break;

  case 8:
#line 172 "parser.y"
                                                                { (yyval.def_gen) = nullptr; }
#line 1750 "parser.cpp"
    break;

  case 9:
#line 173 "parser.y"
                                                                { (yyval.def_gen) = new DefGen((yyvsp[-1].def), (yyvsp[0].def_gen)); }
#line 1756 "parser.cpp"
    break;

  case 10:
#line 177 "parser.y"
                                                                { (yyval.def) = new Def((yyvsp[-3].::std::string), (yyvsp[-2].par_gen), (yyvsp[0].expr), NULL, NULL, false); }
#line 1762 "parser.cpp"
    break;

  case 11:
#line 178 "parser.y"
                                                                { (yyval.def) = new Def((yyvsp[-5].::std::string), (yyvsp[-4].par_gen), (yyvsp[0].expr), (yyvsp[-2].type), nullptr, false); }
#line 1768 "parser.cpp"
    break;

  case 12:
#line 179 "parser.y"
                                                                { (yyval.def) = new Def((yyvsp[0].::std::string), nullptr, nullptr, nullptr, nullptr, true); }
#line 1774 "parser.cpp"
    break;

  case 13:
#line 180 "parser.y"
                                                                { (yyval.def) = new Def((yyvsp[-2].::std::string), nullptr, nullptr, (yyvsp[0].type), nullptr, true); }
#line 1780 "parser.cpp"
    break;

  case 14:
#line 181 "parser.y"
                                                                { (yyval.def) = new Def((yyvsp[-4].::std::string), nullptr, (yyvsp[-2].expr), nullptr, (yyvsp[-1].comma_expr_gen), true); }
#line 1786 "parser.cpp"
    break;

  case 15:
#line 182 "parser.y"
                                                                { (yyval.def) = new Def((yyvsp[-6].::std::string), nullptr, (yyvsp[-4].expr), (yyvsp[0].type), (yyvsp[-3].comma_expr_gen), true); }
#line 1792 "parser.cpp"
    break;

  case 16:
#line 186 "parser.y"
                                                                { (yyval.par_gen) = nullptr; }
#line 1798 "parser.cpp"
    break;

  case 17:
#line 187 "parser.y"
                                                                { (yyval.par_gen) = new ParGen((yyvsp[-1].par), (yyvsp[0].par_gen)); }
#line 1804 "parser.cpp"
    break;

  case 18:
#line 191 "parser.y"
                                                                { (yyval.comma_expr_gen) = nullptr; }
#line 1810 "parser.cpp"
    break;

  case 19:
#line 192 "parser.y"
                                                                { (yyval.comma_expr_gen) = new CommaExprGen((yyvsp[-1].expr), (yyvsp[0].comma_expr_gen)); }
#line 1816 "parser.cpp"
    break;

  case 20:
#line 196 "parser.y"
                                                                { (yyval.type_def) = new TypeDef((yyvsp[-1].tdef), (yyvsp[0].tdef_gen)); }
#line 1822 "parser.cpp"
    break;

  case 21:
#line 200 "parser.y"
                                                                { (yyval.tdef_gen) = nullptr; }
#line 1828 "parser.cpp"
    break;

  case 22:
#line 201 "parser.y"
                                                                { (yyval.tdef_gen) = new TdefGen((yyvsp[-1].tdef), (yyvsp[0].tdef_gen)); }
#line 1834 "parser.cpp"
    break;

  case 23:
#line 205 "parser.y"
                                                                { (yyval.tdef) = new Tdef((yyvsp[-3].::std::string), (yyvsp[-1].constr), (yyvsp[0].bar_constr_gen)); }
#line 1840 "parser.cpp"
    break;

  case 24:
#line 209 "parser.y"
                                                                { (yyval.bar_constr_gen) = nullptr; }
#line 1846 "parser.cpp"
    break;

  case 25:
#line 210 "parser.y"
                                                                { (yyval.bar_constr_gen) = new BarConstrGen((yyvsp[-1].constr), (yyvsp[0].bar_constr_gen)); }
#line 1852 "parser.cpp"
    break;

  case 26:
#line 214 "parser.y"
                                                                { (yyval.constr) = new Constr((yyvsp[0].::std::string), nullptr); }
#line 1858 "parser.cpp"
    break;

  case 27:
#line 215 "parser.y"
                                                                { (yyval.constr) = new Constr((yyvsp[-2].::std::string), (yyvsp[0].type_gen)); }
#line 1864 "parser.cpp"
    break;

  case 28:
#line 219 "parser.y"
                                                                { (yyval.type_gen) = new TypeGen((yyvsp[0].type), nullptr); }
#line 1870 "parser.cpp"
    break;

  case 29:
#line 220 "parser.y"
                                                                { (yyval.type_gen) = new TypeGen((yyvsp[-1].type), (yyvsp[0].type_gen)); }
#line 1876 "parser.cpp"
    break;

  case 30:
#line 224 "parser.y"
                                                                { (yyval.par) = new Par((yyvsp[0].::std::string), nullptr); }
#line 1882 "parser.cpp"
    break;

  case 31:
#line 225 "parser.y"
                                                                { (yyval.par) = new Par((yyvsp[-3].::std::string), (yyvsp[-1].type)); }
#line 1888 "parser.cpp"
    break;

  case 32:
#line 229 "parser.y"
                                                                { (yyval.type) = new Unit(); }
#line 1894 "parser.cpp"
    break;

  case 33:
#line 230 "parser.y"
                                                                { (yyval.type) = new Integer(); }
#line 1900 "parser.cpp"
    break;

  case 34:
#line 231 "parser.y"
                                                                { (yyval.type) = new Character(); }
#line 1906 "parser.cpp"
    break;

  case 35:
#line 232 "parser.y"
                                                                { (yyval.type) = new Boolean(); }
#line 1912 "parser.cpp"
    break;

  case 36:
#line 233 "parser.y"
                                                                { (yyval.type) = new Float(); }
#line 1918 "parser.cpp"
    break;

  case 37:
#line 234 "parser.y"
                                                                { (yyval.type) = (yyvsp[-1].type); }
#line 1924 "parser.cpp"
    break;

  case 38:
#line 235 "parser.y"
                                                                { (yyval.type) = new Function((yyvsp[-2].type), (yyvsp[0].type)); }
#line 1930 "parser.cpp"
    break;

  case 39:
#line 236 "parser.y"
                                                                { (yyval.type) = new Reference((yyvsp[-1].type)); }
#line 1936 "parser.cpp"
    break;

  case 40:
#line 237 "parser.y"
                                                                { (yyval.type) = new Array((yyvsp[0].type), 1); }
#line 1942 "parser.cpp"
    break;

  case 41:
#line 238 "parser.y"
                                                                { (yyval.type) = new Array((yyvsp[0].type), (yyvsp[-3].comma_star_gen)); }
#line 1948 "parser.cpp"
    break;

  case 42:
#line 239 "parser.y"
                                                                { (yyval.type) = new CustomId((yyvsp[0].::std::string)); }
#line 1954 "parser.cpp"
    break;

  case 43:
#line 243 "parser.y"
                                                                { (yyval.comma_star_gen) = 1; }
#line 1960 "parser.cpp"
    break;

  case 44:
#line 244 "parser.y"
                                                                { (yyval.comma_star_gen) = 1 + (yyvsp[0].comma_star_gen); }
#line 1966 "parser.cpp"
    break;

  case 45:
#line 248 "parser.y"
                                                                { (yyval.expr_high) = new UnOp("!", (yyvsp[0].expr_high)); }
#line 1972 "parser.cpp"
    break;

  case 46:
#line 249 "parser.y"
                                                                { (yyval.expr_high) = (yyvsp[-1].expr); }
#line 1978 "parser.cpp"
    break;

  case 47:
#line 250 "parser.y"
                                                                { (yyval.expr_high) = new IntConst((yyvsp[0].num)); }
#line 1984 "parser.cpp"
    break;

  case 48:
#line 251 "parser.y"
                                                                { (yyval.expr_high) = new FloatConst((yyvsp[0].flt)); }
#line 1990 "parser.cpp"
    break;

  case 49:
#line 252 "parser.y"
                                                                { (yyval.expr_high) = new CharConst((yyvsp[0].chr)); }
#line 1996 "parser.cpp"
    break;

  case 50:
#line 253 "parser.y"
                                                                { (yyval.expr_high) = new StringLiteral((yyvsp[0].::std::string)); }
#line 2002 "parser.cpp"
    break;

  case 51:
#line 254 "parser.y"
                                                                { (yyval.expr_high) = new BooleanConst(true); }
#line 2008 "parser.cpp"
    break;

  case 52:
#line 255 "parser.y"
                                                                { (yyval.expr_high) = new BooleanConst(false); }
#line 2014 "parser.cpp"
    break;

  case 53:
#line 256 "parser.y"
                                                                { (yyval.expr_high) = new UnitConst(); }
#line 2020 "parser.cpp"
    break;

  case 54:
#line 257 "parser.y"
                                                                { (yyval.expr_high) = new ArrayItem((yyvsp[-4].::std::string), (yyvsp[-2].expr), (yyvsp[-1].comma_expr_gen)); }
#line 2026 "parser.cpp"
    break;

  case 55:
#line 258 "parser.y"
                                                                { (yyval.expr_high) = new Id((yyvsp[0].::std::string)); }
#line 2032 "parser.cpp"
    break;

  case 56:
#line 259 "parser.y"
                                                                { (yyval.expr_high) = new Constr((yyvsp[0].::std::string), nullptr); }
#line 2038 "parser.cpp"
    break;

  case 57:
#line 263 "parser.y"
                                                                { (yyval.expr) = new UnOp("+", (yyvsp[0].expr)); }
#line 2044 "parser.cpp"
    break;

  case 58:
#line 264 "parser.y"
                                                                { (yyval.expr) = new UnOp("-", (yyvsp[0].expr)); }
#line 2050 "parser.cpp"
    break;

  case 59:
#line 265 "parser.y"
                                                                { (yyval.expr) = new UnOp("+", (yyvsp[0].expr)); }
#line 2056 "parser.cpp"
    break;

  case 60:
#line 266 "parser.y"
                                                                { (yyval.expr) = new UnOp("-", (yyvsp[0].expr)); }
#line 2062 "parser.cpp"
    break;

  case 61:
#line 267 "parser.y"
                                                                { (yyval.expr) = new UnOp("not", (yyvsp[0].expr)); }
#line 2068 "parser.cpp"
    break;

  case 62:
#line 268 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "+", (yyvsp[0].expr)); }
#line 2074 "parser.cpp"
    break;

  case 63:
#line 269 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "-", (yyvsp[0].expr)); }
#line 2080 "parser.cpp"
    break;

  case 64:
#line 270 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "*", (yyvsp[0].expr)); }
#line 2086 "parser.cpp"
    break;

  case 65:
#line 271 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "/", (yyvsp[0].expr)); }
#line 2092 "parser.cpp"
    break;

  case 66:
#line 272 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "+.", (yyvsp[0].expr)); }
#line 2098 "parser.cpp"
    break;

  case 67:
#line 273 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "-.", (yyvsp[0].expr)); }
#line 2104 "parser.cpp"
    break;

  case 68:
#line 274 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "*.", (yyvsp[0].expr)); }
#line 2110 "parser.cpp"
    break;

  case 69:
#line 275 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "/.", (yyvsp[0].expr)); }
#line 2116 "parser.cpp"
    break;

  case 70:
#line 276 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "mod", (yyvsp[0].expr)); }
#line 2122 "parser.cpp"
    break;

  case 71:
#line 277 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "**", (yyvsp[0].expr)); }
#line 2128 "parser.cpp"
    break;

  case 72:
#line 278 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "=", (yyvsp[0].expr)); }
#line 2134 "parser.cpp"
    break;

  case 73:
#line 279 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "<>", (yyvsp[0].expr)); }
#line 2140 "parser.cpp"
    break;

  case 74:
#line 280 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "<", (yyvsp[0].expr)); }
#line 2146 "parser.cpp"
    break;

  case 75:
#line 281 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), ">", (yyvsp[0].expr)); }
#line 2152 "parser.cpp"
    break;

  case 76:
#line 282 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "<=", (yyvsp[0].expr)); }
#line 2158 "parser.cpp"
    break;

  case 77:
#line 283 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), ">=", (yyvsp[0].expr)); }
#line 2164 "parser.cpp"
    break;

  case 78:
#line 284 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "==", (yyvsp[0].expr)); }
#line 2170 "parser.cpp"
    break;

  case 79:
#line 285 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "!=", (yyvsp[0].expr)); }
#line 2176 "parser.cpp"
    break;

  case 80:
#line 286 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "&&", (yyvsp[0].expr)); }
#line 2182 "parser.cpp"
    break;

  case 81:
#line 287 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), "||", (yyvsp[0].expr)); }
#line 2188 "parser.cpp"
    break;

  case 82:
#line 288 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), ";", (yyvsp[0].expr)); }
#line 2194 "parser.cpp"
    break;

  case 83:
#line 289 "parser.y"
                                                                { (yyval.expr) = new BinOp((yyvsp[-2].expr), ":=", (yyvsp[0].expr)); }
#line 2200 "parser.cpp"
    break;

  case 84:
#line 290 "parser.y"
                                                                { (yyval.expr) = new Id((yyvsp[-2].::std::string), (yyvsp[-1].expr_high), (yyvsp[0].expr_high_gen)); }
#line 2206 "parser.cpp"
    break;

  case 85:
#line 291 "parser.y"
                                                                { (yyval.expr) = new Constr((yyvsp[-2].::std::string), (yyvsp[-1].expr_high), (yyvsp[0].expr_high_gen)); }
#line 2212 "parser.cpp"
    break;

  case 86:
#line 292 "parser.y"
                                                                { (yyval.expr) = new Dim((yyvsp[0].::std::string), nullptr); }
#line 2218 "parser.cpp"
    break;

  case 87:
#line 293 "parser.y"
                                                                { (yyval.expr) = new Dim((yyvsp[0].::std::string), (yyvsp[-1].num)); }
#line 2224 "parser.cpp"
    break;

  case 88:
#line 294 "parser.y"
                                                                { (yyval.expr) = new New((yyvsp[0].type)); }
#line 2230 "parser.cpp"
    break;

  case 89:
#line 295 "parser.y"
                                                                { (yyval.expr) = new Delete((yyvsp[0].expr)); }
#line 2236 "parser.cpp"
    break;

  case 90:
#line 296 "parser.y"
                                                                { (yyval.expr) = new LetIn((yyvsp[-2].letdef), (yyvsp[0].expr)); }
#line 2242 "parser.cpp"
    break;

  case 91:
#line 297 "parser.y"
                                                                { (yyval.expr) = new Begin((yyvsp[-1].expr)); }
#line 2248 "parser.cpp"
    break;

  case 92:
#line 298 "parser.y"
                                                                { (yyval.expr) = new If((yyvsp[-2].expr), (yyvsp[0].expr), nullptr); }
#line 2254 "parser.cpp"
    break;

  case 93:
#line 299 "parser.y"
                                                                { (yyval.expr) = new If((yyvsp[-4].expr), (yyvsp[-2].expr), (yyvsp[0].expr)); }
#line 2260 "parser.cpp"
    break;

  case 94:
#line 300 "parser.y"
                                                                { (yyval.expr) = new While((yyvsp[-3].expr), (yyvsp[-1].expr)); }
#line 2266 "parser.cpp"
    break;

  case 95:
#line 301 "parser.y"
                                                                { (yyval.expr) = new For((yyvsp[-7].::std::string), (yyvsp[-5].expr), (yyvsp[-3].expr), (yyvsp[-1].expr), true); }
#line 2272 "parser.cpp"
    break;

  case 96:
#line 302 "parser.y"
                                                                { (yyval.expr) = new For((yyvsp[-7].::std::string), (yyvsp[-5].expr), (yyvsp[-3].expr), (yyvsp[-1].expr), false); }
#line 2278 "parser.cpp"
    break;

  case 97:
#line 303 "parser.y"
                                                                { (yyval.expr) = new Match((yyvsp[-4].expr), (yyvsp[-2].clause), (yyvsp[-1].bar_clause_gen)); }
#line 2284 "parser.cpp"
    break;

  case 98:
#line 304 "parser.y"
                                                                { (yyval.expr) = (yyvsp[0].expr_high); }
#line 2290 "parser.cpp"
    break;

  case 99:
#line 308 "parser.y"
                                                                { (yyval.expr_high_gen) = nullptr; }
#line 2296 "parser.cpp"
    break;

  case 100:
#line 309 "parser.y"
                                                                { (yyval.expr_high_gen) = new ExprGen((yyvsp[-1].expr_high), (yyvsp[0].expr_high_gen)); }
#line 2302 "parser.cpp"
    break;

  case 101:
#line 313 "parser.y"
                                                                { (yyval.bar_clause_gen) = nullptr; }
#line 2308 "parser.cpp"
    break;

  case 102:
#line 314 "parser.y"
                                                                { (yyval.bar_clause_gen) = new BarClauseGen((yyvsp[-1].clause), (yyvsp[0].bar_clause_gen)); }
#line 2314 "parser.cpp"
    break;

  case 103:
#line 318 "parser.y"
                                                                { (yyval.clause) = new Clause((yyvsp[-2].pattern), (yyvsp[0].expr)); }
#line 2320 "parser.cpp"
    break;

  case 104:
#line 322 "parser.y"
                                                                { (yyval.pattern) = (yyvsp[0].pattern_high); }
#line 2326 "parser.cpp"
    break;

  case 105:
#line 323 "parser.y"
                                                                { (yyval.pattern) = PatternConstr((yyvsp[-1].::std::string), (yyvsp[0].pattern_high_gen)); }
#line 2332 "parser.cpp"
    break;

  case 106:
#line 327 "parser.y"
                                                                { (yyval.pattern_high) = new IntConst((yyvsp[0].num), '+'); }
#line 2338 "parser.cpp"
    break;

  case 107:
#line 328 "parser.y"
                                                                { (yyval.pattern_high) = new IntConst((yyvsp[0].num), '-'); }
#line 2344 "parser.cpp"
    break;

  case 108:
#line 329 "parser.y"
                                                                { (yyval.pattern_high) = new FloatConst((yyvsp[0].flt), "+."); }
#line 2350 "parser.cpp"
    break;

  case 109:
#line 330 "parser.y"
                                                                { (yyval.pattern_high) = new FloatConst((yyvsp[0].flt), "-."); }
#line 2356 "parser.cpp"
    break;

  case 110:
#line 331 "parser.y"
                                                                { (yyval.pattern_high) = new CharConst((yyvsp[0].chr)); }
#line 2362 "parser.cpp"
    break;

  case 111:
#line 332 "parser.y"
                                                                { (yyval.pattern_high) = new BooleanConst(true); }
#line 2368 "parser.cpp"
    break;

  case 112:
#line 333 "parser.y"
                                                                { (yyval.pattern_high) = new BooleanConst(false); }
#line 2374 "parser.cpp"
    break;

  case 113:
#line 334 "parser.y"
                                                                { (yyval.pattern_high) = new Id((yyvsp[0].::std::string)); }
#line 2380 "parser.cpp"
    break;

  case 114:
#line 335 "parser.y"
                                                                { (yyval.pattern_high) = (yyvsp[-1].pattern); }
#line 2386 "parser.cpp"
    break;

  case 115:
#line 339 "parser.y"
                                                                { (yyval.pattern_high_gen) = nullptr; }
#line 2392 "parser.cpp"
    break;

  case 116:
#line 340 "parser.y"
                                                                { (yyval.pattern_high_gen) = new PatternGen((yyvsp[-1].pattern_high), (yyvsp[0].pattern_high_gen)); }
#line 2398 "parser.cpp"
    break;


#line 2402 "parser.cpp"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[+*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 343 "parser.y"


int main() {
  yydebug = 1;
  int result = yyparse();
  if (result == 0) printf("Success.\n");
  return result;
}
