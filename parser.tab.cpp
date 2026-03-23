/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

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

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "ast.h"
#include "symtab.h"
#include "ir.h"

void yyerror(const char *s);
int yylex();

#line 84 "parser.tab.c"

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

#include "parser.tab.h"
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_INT32 = 3,                      /* INT32  */
  YYSYMBOL_INT64 = 4,                      /* INT64  */
  YYSYMBOL_INT128 = 5,                     /* INT128  */
  YYSYMBOL_FLOAT = 6,                      /* FLOAT  */
  YYSYMBOL_CHAR = 7,                       /* CHAR  */
  YYSYMBOL_BOOL = 8,                       /* BOOL  */
  YYSYMBOL_VOID_KW = 9,                    /* VOID_KW  */
  YYSYMBOL_IF = 10,                        /* IF  */
  YYSYMBOL_ELSE = 11,                      /* ELSE  */
  YYSYMBOL_WHILE = 12,                     /* WHILE  */
  YYSYMBOL_FOR = 13,                       /* FOR  */
  YYSYMBOL_FUNC = 14,                      /* FUNC  */
  YYSYMBOL_RETURN = 15,                    /* RETURN  */
  YYSYMBOL_ID = 16,                        /* ID  */
  YYSYMBOL_NUMBER = 17,                    /* NUMBER  */
  YYSYMBOL_PLUS = 18,                      /* PLUS  */
  YYSYMBOL_MINUS = 19,                     /* MINUS  */
  YYSYMBOL_MUL = 20,                       /* MUL  */
  YYSYMBOL_DIV = 21,                       /* DIV  */
  YYSYMBOL_AND = 22,                       /* AND  */
  YYSYMBOL_OR = 23,                        /* OR  */
  YYSYMBOL_NOT = 24,                       /* NOT  */
  YYSYMBOL_ASSIGN = 25,                    /* ASSIGN  */
  YYSYMBOL_LT = 26,                        /* LT  */
  YYSYMBOL_GT = 27,                        /* GT  */
  YYSYMBOL_LE = 28,                        /* LE  */
  YYSYMBOL_GE = 29,                        /* GE  */
  YYSYMBOL_EQ = 30,                        /* EQ  */
  YYSYMBOL_NEQ = 31,                       /* NEQ  */
  YYSYMBOL_SEMICOLON = 32,                 /* SEMICOLON  */
  YYSYMBOL_COMMA = 33,                     /* COMMA  */
  YYSYMBOL_LBRACE = 34,                    /* LBRACE  */
  YYSYMBOL_RBRACE = 35,                    /* RBRACE  */
  YYSYMBOL_LPAREN = 36,                    /* LPAREN  */
  YYSYMBOL_RPAREN = 37,                    /* RPAREN  */
  YYSYMBOL_LBRACKET = 38,                  /* LBRACKET  */
  YYSYMBOL_RBRACKET = 39,                  /* RBRACKET  */
  YYSYMBOL_YYACCEPT = 40,                  /* $accept  */
  YYSYMBOL_program = 41,                   /* program  */
  YYSYMBOL_type_kw = 42,                   /* type_kw  */
  YYSYMBOL_ret_type_kw = 43,               /* ret_type_kw  */
  YYSYMBOL_body = 44,                      /* body  */
  YYSYMBOL_body_stmt = 45,                 /* body_stmt  */
  YYSYMBOL_body_assignment = 46,           /* body_assignment  */
  YYSYMBOL_statement = 47,                 /* statement  */
  YYSYMBOL_declaration = 48,               /* declaration  */
  YYSYMBOL_assignment = 49,                /* assignment  */
  YYSYMBOL_param_list = 50,                /* param_list  */
  YYSYMBOL_param_list_ne = 51,             /* param_list_ne  */
  YYSYMBOL_arg_list = 52,                  /* arg_list  */
  YYSYMBOL_arg_list_ne = 53,               /* arg_list_ne  */
  YYSYMBOL_func_def = 54,                  /* func_def  */
  YYSYMBOL_func_call_stmt = 55,            /* func_call_stmt  */
  YYSYMBOL_return_stmt = 56,               /* return_stmt  */
  YYSYMBOL_for_stmt = 57,                  /* for_stmt  */
  YYSYMBOL_while_stmt = 58,                /* while_stmt  */
  YYSYMBOL_if_stmt = 59,                   /* if_stmt  */
  YYSYMBOL_expression = 60                 /* expression  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




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

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
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
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
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

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

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
#define YYFINAL  37
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   459

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  40
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  21
/* YYNRULES -- Number of rules.  */
#define YYNRULES  67
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  152

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   294


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
      35,    36,    37,    38,    39
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,    63,    63,    64,    68,    69,    70,    74,    75,    76,
      77,    81,    82,    91,    92,    93,    94,    95,    96,    97,
     102,   107,   116,   117,   118,   119,   120,   121,   122,   123,
     127,   133,   139,   150,   157,   168,   169,   173,   178,   186,
     187,   191,   196,   204,   216,   226,   232,   241,   274,   284,
     291,   301,   302,   303,   304,   305,   306,   307,   308,   309,
     310,   311,   312,   313,   314,   319,   320,   321
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "INT32", "INT64",
  "INT128", "FLOAT", "CHAR", "BOOL", "VOID_KW", "IF", "ELSE", "WHILE",
  "FOR", "FUNC", "RETURN", "ID", "NUMBER", "PLUS", "MINUS", "MUL", "DIV",
  "AND", "OR", "NOT", "ASSIGN", "LT", "GT", "LE", "GE", "EQ", "NEQ",
  "SEMICOLON", "COMMA", "LBRACE", "RBRACE", "LPAREN", "RPAREN", "LBRACKET",
  "RBRACKET", "$accept", "program", "type_kw", "ret_type_kw", "body",
  "body_stmt", "body_assignment", "statement", "declaration", "assignment",
  "param_list", "param_list_ne", "arg_list", "arg_list_ne", "func_def",
  "func_call_stmt", "return_stmt", "for_stmt", "while_stmt", "if_stmt",
  "expression", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-74)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     431,   -74,   -74,   -74,   -33,   -29,   -26,   107,   -15,   -20,
     255,    11,   -74,   -74,   -74,   -74,   -74,   -74,   -74,   -74,
     -74,    14,    14,    20,   -74,   -74,   -74,   -74,    37,   -32,
     -74,    14,   -74,   254,    14,    14,    14,   -74,   -74,   -17,
     180,   200,    30,    33,    14,    14,   -74,    14,    14,    14,
      14,    14,    14,    14,    14,    14,    14,    14,    14,   -74,
     269,    24,    42,   374,   138,    14,   -74,    66,    52,    59,
      14,    29,    58,   152,    71,    71,   -74,   -74,   402,   388,
     127,   127,   127,   127,   430,   430,   -74,    57,    14,    75,
     284,    62,   -74,   -74,   299,    86,    70,    80,   -74,   -74,
     -74,   374,    14,   -74,    82,    55,    69,    14,   -74,    81,
      29,   314,   -74,   -14,   111,   -74,   -74,   -74,   -74,   -74,
     -74,   -74,   -74,   -74,   329,   -74,   109,   -74,    14,    14,
      89,   120,    93,   -74,   344,   166,   -74,   112,   -74,   -74,
     117,   114,    14,    14,   -74,   220,   359,   105,   -74,   -74,
     128,   -74
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int8 yydefact[] =
{
       0,     4,     5,     6,     0,     0,     0,     0,     0,     0,
       0,     0,     3,    22,    23,    27,    28,    29,    24,    25,
      26,     0,     0,     0,     7,     8,     9,    10,     0,    66,
      67,     0,    46,     0,     0,    39,     0,     1,     2,     0,
       0,     0,     0,     0,    39,     0,    63,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,    45,
       0,     0,    40,    41,     0,     0,    30,     0,     0,     0,
       0,    35,     0,     0,    51,    52,    53,    54,    61,    62,
      55,    56,    57,    58,    59,    60,    33,     0,     0,     0,
       0,     0,    11,    11,     0,     0,     0,    36,    64,    65,
      44,    42,     0,    32,     0,     0,     0,     0,    37,     0,
       0,     0,    31,     0,    49,    12,    14,    13,    18,    19,
      15,    16,    17,    48,     0,    11,     0,    34,     0,     0,
       0,     0,     0,    38,     0,     0,    11,     0,    43,    20,
       0,     0,     0,     0,    50,     0,     0,     0,    21,    11,
       0,    47
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -74,   -74,   -59,   -74,   -73,   -74,   -74,   140,    19,   -74,
     -74,   -74,   108,   -74,   -74,    25,    54,    56,    77,    78,
      -8
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
       0,    10,    11,    28,   105,   115,   116,    12,   117,    14,
      96,    97,    61,    62,    15,   118,   119,   120,   121,   122,
      63
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      33,    29,    30,    21,    44,    34,    45,    22,    65,    31,
      23,   128,    95,    40,    41,    66,    35,    32,    36,    13,
     106,    67,    35,    46,   129,    16,    60,    39,    64,    13,
      29,    30,     1,     2,     3,    16,    42,    73,    31,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,   126,   132,    43,    17,    70,    18,    90,     1,     2,
       3,    87,    94,   141,    17,     4,    18,     5,     6,    71,
       8,   113,     1,     2,     3,    88,   150,    19,    20,     4,
     101,     5,     6,    91,     8,   113,    92,    19,    20,   100,
     114,    49,    50,    93,   111,    98,     1,     2,     3,   124,
     102,   104,   108,     4,   123,     5,     6,   109,     8,   113,
      24,    25,    26,   110,   112,   125,    27,     1,     2,     3,
     134,   135,   130,   136,     4,   133,     5,     6,   138,     8,
     113,     1,     2,     3,   145,   146,   137,   142,     4,   149,
       5,     6,   143,     8,   113,    47,    48,    49,    50,   144,
      38,     0,    72,     0,     0,     0,    47,    48,    49,    50,
      51,    52,     0,   151,    53,    54,    55,    56,    57,    58,
      47,    48,    49,    50,    51,    52,     0,    89,    53,    54,
      55,    56,    57,    58,    47,    48,    49,    50,    51,    52,
       0,    99,    53,    54,    55,    56,    57,    58,    47,    48,
      49,    50,    51,    52,     0,   140,    53,    54,    55,    56,
      57,    58,     0,     0,     0,     0,     0,    68,    47,    48,
      49,    50,    51,    52,     0,     0,    53,    54,    55,    56,
      57,    58,     0,     0,     0,     0,     0,    69,    47,    48,
      49,    50,    51,    52,     0,     0,    53,    54,    55,    56,
      57,    58,     0,     0,     0,    37,     0,   147,     1,     2,
       3,     0,     0,     0,     0,     4,     0,     5,     6,     7,
       8,     9,    47,    48,    49,    50,    51,    52,     0,     0,
      53,    54,    55,    56,    57,    58,    59,    47,    48,    49,
      50,    51,    52,     0,     0,    53,    54,    55,    56,    57,
      58,    86,    47,    48,    49,    50,    51,    52,     0,     0,
      53,    54,    55,    56,    57,    58,   103,    47,    48,    49,
      50,    51,    52,     0,     0,    53,    54,    55,    56,    57,
      58,   107,    47,    48,    49,    50,    51,    52,     0,     0,
      53,    54,    55,    56,    57,    58,   127,    47,    48,    49,
      50,    51,    52,     0,     0,    53,    54,    55,    56,    57,
      58,   131,    47,    48,    49,    50,    51,    52,     0,     0,
      53,    54,    55,    56,    57,    58,   139,    47,    48,    49,
      50,    51,    52,     0,     0,    53,    54,    55,    56,    57,
      58,   148,    47,    48,    49,    50,    51,    52,     0,     0,
      53,    54,    55,    56,    57,    58,    47,    48,    49,    50,
      51,     0,     0,     0,    53,    54,    55,    56,    57,    58,
      47,    48,    49,    50,     0,     0,     0,     0,    53,    54,
      55,    56,    57,    58,     1,     2,     3,     0,     0,     0,
       0,     4,     0,     5,     6,     7,     8,     9,    47,    48,
      49,    50,     0,     0,     0,     0,    53,    54,    55,    56
};

static const yytype_int16 yycheck[] =
{
       8,    16,    17,    36,    36,    25,    38,    36,    25,    24,
      36,    25,    71,    21,    22,    32,    36,    32,    38,     0,
      93,    38,    36,    31,    38,     0,    34,    16,    36,    10,
      16,    17,     3,     4,     5,    10,    16,    45,    24,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,   110,   125,    16,     0,    25,     0,    65,     3,     4,
       5,    37,    70,   136,    10,    10,    10,    12,    13,    36,
      15,    16,     3,     4,     5,    33,   149,     0,     0,    10,
      88,    12,    13,    17,    15,    16,    34,    10,    10,    32,
      35,    20,    21,    34,   102,    37,     3,     4,     5,   107,
      25,    39,    16,    10,    35,    12,    13,    37,    15,    16,
       3,     4,     5,    33,    32,    34,     9,     3,     4,     5,
     128,   129,    11,    34,    10,    16,    12,    13,    35,    15,
      16,     3,     4,     5,   142,   143,    16,    25,    10,    34,
      12,    13,    25,    15,    16,    18,    19,    20,    21,    35,
      10,    -1,    44,    -1,    -1,    -1,    18,    19,    20,    21,
      22,    23,    -1,    35,    26,    27,    28,    29,    30,    31,
      18,    19,    20,    21,    22,    23,    -1,    39,    26,    27,
      28,    29,    30,    31,    18,    19,    20,    21,    22,    23,
      -1,    39,    26,    27,    28,    29,    30,    31,    18,    19,
      20,    21,    22,    23,    -1,    39,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    -1,    -1,    37,    18,    19,
      20,    21,    22,    23,    -1,    -1,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,    -1,    -1,    37,    18,    19,
      20,    21,    22,    23,    -1,    -1,    26,    27,    28,    29,
      30,    31,    -1,    -1,    -1,     0,    -1,    37,     3,     4,
       5,    -1,    -1,    -1,    -1,    10,    -1,    12,    13,    14,
      15,    16,    18,    19,    20,    21,    22,    23,    -1,    -1,
      26,    27,    28,    29,    30,    31,    32,    18,    19,    20,
      21,    22,    23,    -1,    -1,    26,    27,    28,    29,    30,
      31,    32,    18,    19,    20,    21,    22,    23,    -1,    -1,
      26,    27,    28,    29,    30,    31,    32,    18,    19,    20,
      21,    22,    23,    -1,    -1,    26,    27,    28,    29,    30,
      31,    32,    18,    19,    20,    21,    22,    23,    -1,    -1,
      26,    27,    28,    29,    30,    31,    32,    18,    19,    20,
      21,    22,    23,    -1,    -1,    26,    27,    28,    29,    30,
      31,    32,    18,    19,    20,    21,    22,    23,    -1,    -1,
      26,    27,    28,    29,    30,    31,    32,    18,    19,    20,
      21,    22,    23,    -1,    -1,    26,    27,    28,    29,    30,
      31,    32,    18,    19,    20,    21,    22,    23,    -1,    -1,
      26,    27,    28,    29,    30,    31,    18,    19,    20,    21,
      22,    -1,    -1,    -1,    26,    27,    28,    29,    30,    31,
      18,    19,    20,    21,    -1,    -1,    -1,    -1,    26,    27,
      28,    29,    30,    31,     3,     4,     5,    -1,    -1,    -1,
      -1,    10,    -1,    12,    13,    14,    15,    16,    18,    19,
      20,    21,    -1,    -1,    -1,    -1,    26,    27,    28,    29
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,     3,     4,     5,    10,    12,    13,    14,    15,    16,
      41,    42,    47,    48,    49,    54,    55,    56,    57,    58,
      59,    36,    36,    36,     3,     4,     5,     9,    43,    16,
      17,    24,    32,    60,    25,    36,    38,     0,    47,    16,
      60,    60,    16,    16,    36,    38,    60,    18,    19,    20,
      21,    22,    23,    26,    27,    28,    29,    30,    31,    32,
      60,    52,    53,    60,    60,    25,    32,    38,    37,    37,
      25,    36,    52,    60,    60,    60,    60,    60,    60,    60,
      60,    60,    60,    60,    60,    60,    32,    37,    33,    39,
      60,    17,    34,    34,    60,    42,    50,    51,    37,    39,
      32,    60,    25,    32,    39,    44,    44,    32,    16,    37,
      33,    60,    32,    16,    35,    45,    46,    48,    55,    56,
      57,    58,    59,    35,    60,    34,    42,    32,    25,    38,
      11,    32,    44,    16,    60,    60,    34,    16,    35,    32,
      39,    44,    25,    25,    35,    60,    60,    37,    32,    34,
      44,    35
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    40,    41,    41,    42,    42,    42,    43,    43,    43,
      43,    44,    44,    45,    45,    45,    45,    45,    45,    45,
      46,    46,    47,    47,    47,    47,    47,    47,    47,    47,
      48,    48,    48,    49,    49,    50,    50,    51,    51,    52,
      52,    53,    53,    54,    55,    56,    56,    57,    58,    59,
      59,    60,    60,    60,    60,    60,    60,    60,    60,    60,
      60,    60,    60,    60,    60,    60,    60,    60
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     2,     1,     1,     1,     1,     1,     1,     1,
       4,     7,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     6,     5,     4,     7,     0,     1,     2,     4,     0,
       1,     1,     3,     9,     5,     3,     2,    15,     7,     7,
      11,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     4,     4,     1,     1
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


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

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


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




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
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
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
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
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
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
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

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
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
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
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  case 4: /* type_kw: INT32  */
#line 68 "parser.y"
              { (yyval.id) = (char*)"int32";  }
#line 1286 "parser.tab.c"
    break;

  case 5: /* type_kw: INT64  */
#line 69 "parser.y"
              { (yyval.id) = (char*)"int64";  }
#line 1292 "parser.tab.c"
    break;

  case 6: /* type_kw: INT128  */
#line 70 "parser.y"
              { (yyval.id) = (char*)"int128"; }
#line 1298 "parser.tab.c"
    break;

  case 7: /* ret_type_kw: INT32  */
#line 74 "parser.y"
               { (yyval.id) = (char*)"int32";  }
#line 1304 "parser.tab.c"
    break;

  case 8: /* ret_type_kw: INT64  */
#line 75 "parser.y"
               { (yyval.id) = (char*)"int64";  }
#line 1310 "parser.tab.c"
    break;

  case 9: /* ret_type_kw: INT128  */
#line 76 "parser.y"
               { (yyval.id) = (char*)"int128"; }
#line 1316 "parser.tab.c"
    break;

  case 10: /* ret_type_kw: VOID_KW  */
#line 77 "parser.y"
               { (yyval.id) = (char*)"void";   }
#line 1322 "parser.tab.c"
    break;

  case 11: /* body: %empty  */
#line 81 "parser.y"
                  { (yyval.stmtlist) = new StatementListNode(); }
#line 1328 "parser.tab.c"
    break;

  case 12: /* body: body body_stmt  */
#line 82 "parser.y"
                     {
            (yyval.stmtlist) = (yyvsp[-1].stmtlist);
            (yyval.stmtlist)->add((yyvsp[0].node));
        }
#line 1337 "parser.tab.c"
    break;

  case 13: /* body_stmt: declaration  */
#line 91 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1343 "parser.tab.c"
    break;

  case 14: /* body_stmt: body_assignment  */
#line 92 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1349 "parser.tab.c"
    break;

  case 15: /* body_stmt: for_stmt  */
#line 93 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1355 "parser.tab.c"
    break;

  case 16: /* body_stmt: while_stmt  */
#line 94 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1361 "parser.tab.c"
    break;

  case 17: /* body_stmt: if_stmt  */
#line 95 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1367 "parser.tab.c"
    break;

  case 18: /* body_stmt: func_call_stmt  */
#line 96 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1373 "parser.tab.c"
    break;

  case 19: /* body_stmt: return_stmt  */
#line 97 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1379 "parser.tab.c"
    break;

  case 20: /* body_assignment: ID ASSIGN expression SEMICOLON  */
#line 103 "parser.y"
        {
            if (!symtab.exists((yyvsp[-3].id))) { printf("Error: %s not declared\n",(yyvsp[-3].id)); exit(1); }
            (yyval.node) = new AssignmentNode((yyvsp[-3].id), (yyvsp[-1].node));
        }
#line 1388 "parser.tab.c"
    break;

  case 21: /* body_assignment: ID LBRACKET expression RBRACKET ASSIGN expression SEMICOLON  */
#line 108 "parser.y"
        {
            if (!symtab.exists((yyvsp[-6].id))) { printf("Error: %s not declared\n",(yyvsp[-6].id)); exit(1); }
            if (!symtab.get((yyvsp[-6].id)).isArray) { printf("Error: %s is not an array\n",(yyvsp[-6].id)); exit(1); }
            (yyval.node) = new ArrayElementAssignmentNode((yyvsp[-6].id), (yyvsp[-4].node), (yyvsp[-1].node));
        }
#line 1398 "parser.tab.c"
    break;

  case 22: /* statement: declaration  */
#line 116 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1404 "parser.tab.c"
    break;

  case 23: /* statement: assignment  */
#line 117 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1410 "parser.tab.c"
    break;

  case 24: /* statement: for_stmt  */
#line 118 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1416 "parser.tab.c"
    break;

  case 25: /* statement: while_stmt  */
#line 119 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1422 "parser.tab.c"
    break;

  case 26: /* statement: if_stmt  */
#line 120 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1428 "parser.tab.c"
    break;

  case 27: /* statement: func_def  */
#line 121 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1434 "parser.tab.c"
    break;

  case 28: /* statement: func_call_stmt  */
#line 122 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1440 "parser.tab.c"
    break;

  case 29: /* statement: return_stmt  */
#line 123 "parser.y"
                       { (yyval.node) = (yyvsp[0].node); }
#line 1446 "parser.tab.c"
    break;

  case 30: /* declaration: type_kw ID SEMICOLON  */
#line 128 "parser.y"
        {
            if (symtab.exists((yyvsp[-1].id))) { printf("Error: redeclaration of %s\n",(yyvsp[-1].id)); exit(1); }
            symtab.insert((yyvsp[-1].id), (yyvsp[-2].id), false, 0);
            (yyval.node) = nullptr;
        }
#line 1456 "parser.tab.c"
    break;

  case 31: /* declaration: type_kw ID LBRACKET NUMBER RBRACKET SEMICOLON  */
#line 134 "parser.y"
        {
            if (symtab.exists((yyvsp[-4].id))) { printf("Error: redeclaration of %s\n",(yyvsp[-4].id)); exit(1); }
            symtab.insert((yyvsp[-4].id), (yyvsp[-5].id), true, (yyvsp[-2].num));
            (yyval.node) = nullptr;
        }
#line 1466 "parser.tab.c"
    break;

  case 32: /* declaration: type_kw ID ASSIGN expression SEMICOLON  */
#line 140 "parser.y"
        {
            if (symtab.exists((yyvsp[-3].id))) { printf("Error: redeclaration of %s\n",(yyvsp[-3].id)); exit(1); }
            symtab.insert((yyvsp[-3].id), (yyvsp[-4].id), false, 0);
            AssignmentNode* a = new AssignmentNode((yyvsp[-3].id), (yyvsp[-1].node));
            a->generateIR();
            (yyval.node) = a;
        }
#line 1478 "parser.tab.c"
    break;

  case 33: /* assignment: ID ASSIGN expression SEMICOLON  */
#line 151 "parser.y"
        {
            if (!symtab.exists((yyvsp[-3].id))) { printf("Error: %s not declared\n",(yyvsp[-3].id)); exit(1); }
            AssignmentNode* a = new AssignmentNode((yyvsp[-3].id), (yyvsp[-1].node));
            a->generateIR();
            (yyval.node) = a;
        }
#line 1489 "parser.tab.c"
    break;

  case 34: /* assignment: ID LBRACKET expression RBRACKET ASSIGN expression SEMICOLON  */
#line 158 "parser.y"
        {
            if (!symtab.exists((yyvsp[-6].id))) { printf("Error: %s not declared\n",(yyvsp[-6].id)); exit(1); }
            if (!symtab.get((yyvsp[-6].id)).isArray) { printf("Error: %s is not an array\n",(yyvsp[-6].id)); exit(1); }
            ArrayElementAssignmentNode* a = new ArrayElementAssignmentNode((yyvsp[-6].id), (yyvsp[-4].node), (yyvsp[-1].node));
            a->generateIR();
            (yyval.node) = a;
        }
#line 1501 "parser.tab.c"
    break;

  case 35: /* param_list: %empty  */
#line 168 "parser.y"
                    { (yyval.paramlist) = new std::vector<ParamNode>(); }
#line 1507 "parser.tab.c"
    break;

  case 36: /* param_list: param_list_ne  */
#line 169 "parser.y"
                    { (yyval.paramlist) = (yyvsp[0].paramlist); }
#line 1513 "parser.tab.c"
    break;

  case 37: /* param_list_ne: type_kw ID  */
#line 174 "parser.y"
        {
            (yyval.paramlist) = new std::vector<ParamNode>();
            (yyval.paramlist)->push_back({std::string((yyvsp[-1].id)), std::string((yyvsp[0].id))});
        }
#line 1522 "parser.tab.c"
    break;

  case 38: /* param_list_ne: param_list_ne COMMA type_kw ID  */
#line 179 "parser.y"
        {
            (yyval.paramlist) = (yyvsp[-3].paramlist);
            (yyval.paramlist)->push_back({std::string((yyvsp[-1].id)), std::string((yyvsp[0].id))});
        }
#line 1531 "parser.tab.c"
    break;

  case 39: /* arg_list: %empty  */
#line 186 "parser.y"
                   { (yyval.arglist) = new std::vector<ASTNode*>(); }
#line 1537 "parser.tab.c"
    break;

  case 40: /* arg_list: arg_list_ne  */
#line 187 "parser.y"
                   { (yyval.arglist) = (yyvsp[0].arglist); }
#line 1543 "parser.tab.c"
    break;

  case 41: /* arg_list_ne: expression  */
#line 192 "parser.y"
        {
            (yyval.arglist) = new std::vector<ASTNode*>();
            (yyval.arglist)->push_back((yyvsp[0].node));
        }
#line 1552 "parser.tab.c"
    break;

  case 42: /* arg_list_ne: arg_list_ne COMMA expression  */
#line 197 "parser.y"
        {
            (yyval.arglist) = (yyvsp[-2].arglist);
            (yyval.arglist)->push_back((yyvsp[0].node));
        }
#line 1561 "parser.tab.c"
    break;

  case 43: /* func_def: FUNC ret_type_kw ID LPAREN param_list RPAREN LBRACE body RBRACE  */
#line 205 "parser.y"
        {
            FunctionDefNode* f = new FunctionDefNode(
                std::string((yyvsp[-7].id)), std::string((yyvsp[-6].id)), *(yyvsp[-4].paramlist), (yyvsp[-1].stmtlist));
            delete (yyvsp[-4].paramlist);
            f->print(0);
            f->generateIR();
            (yyval.node) = f;
        }
#line 1574 "parser.tab.c"
    break;

  case 44: /* func_call_stmt: ID LPAREN arg_list RPAREN SEMICOLON  */
#line 217 "parser.y"
        {
            FunctionCallNode* fc = new FunctionCallNode(std::string((yyvsp[-4].id)), *(yyvsp[-2].arglist));
            delete (yyvsp[-2].arglist);
            fc->generateIR();
            (yyval.node) = fc;
        }
#line 1585 "parser.tab.c"
    break;

  case 45: /* return_stmt: RETURN expression SEMICOLON  */
#line 227 "parser.y"
        {
            ReturnNode* r = new ReturnNode((yyvsp[-1].node));
            r->generateIR();
            (yyval.node) = r;
        }
#line 1595 "parser.tab.c"
    break;

  case 46: /* return_stmt: RETURN SEMICOLON  */
#line 233 "parser.y"
        {
            ReturnNode* r = new ReturnNode(nullptr);
            r->generateIR();
            (yyval.node) = r;
        }
#line 1605 "parser.tab.c"
    break;

  case 47: /* for_stmt: FOR LPAREN ID ASSIGN expression SEMICOLON expression SEMICOLON ID ASSIGN expression RPAREN LBRACE body RBRACE  */
#line 246 "parser.y"
        {
            if (!symtab.exists((yyvsp[-12].id)))     { printf("Error: %s not declared\n", (yyvsp[-12].id));     exit(1); }
            if (!symtab.exists((yyvsp[-6].id))) { printf("Error: %s not declared\n", (yyvsp[-6].id)); exit(1); }

            AssignmentNode*    initNode = new AssignmentNode(std::string((yyvsp[-12].id)),      (yyvsp[-10].node));
            ASTNode*           condNode = (yyvsp[-8].node);
            AssignmentNode*    stepNode = new AssignmentNode(std::string((yyvsp[-6].id)),  (yyvsp[-4].node));
            StatementListNode* bodyNode = (yyvsp[-1].stmtlist);

            ForNode* f = new ForNode(initNode, condNode, stepNode, bodyNode);
            f->print(0);
            f->generateIR();
            (yyval.node) = f;
        }
#line 1624 "parser.tab.c"
    break;

  case 48: /* while_stmt: WHILE LPAREN expression RPAREN LBRACE body RBRACE  */
#line 275 "parser.y"
        {
            WhileNode* w = new WhileNode((yyvsp[-4].node), (yyvsp[-1].stmtlist));
            w->print(0);
            w->generateIR();
            (yyval.node) = w;
        }
#line 1635 "parser.tab.c"
    break;

  case 49: /* if_stmt: IF LPAREN expression RPAREN LBRACE body RBRACE  */
#line 285 "parser.y"
        {
            IfNode* n = new IfNode((yyvsp[-4].node), (yyvsp[-1].stmtlist), nullptr);
            n->print(0);
            n->generateIR();
            (yyval.node) = n;
        }
#line 1646 "parser.tab.c"
    break;

  case 50: /* if_stmt: IF LPAREN expression RPAREN LBRACE body RBRACE ELSE LBRACE body RBRACE  */
#line 292 "parser.y"
        {
            IfNode* n = new IfNode((yyvsp[-8].node), (yyvsp[-5].stmtlist), (yyvsp[-1].stmtlist));
            n->print(0);
            n->generateIR();
            (yyval.node) = n;
        }
#line 1657 "parser.tab.c"
    break;

  case 51: /* expression: expression PLUS expression  */
#line 301 "parser.y"
                                   { (yyval.node) = new BinaryOpNode("+",(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1663 "parser.tab.c"
    break;

  case 52: /* expression: expression MINUS expression  */
#line 302 "parser.y"
                                   { (yyval.node) = new BinaryOpNode("-",(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1669 "parser.tab.c"
    break;

  case 53: /* expression: expression MUL expression  */
#line 303 "parser.y"
                                   { (yyval.node) = new BinaryOpNode("*",(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1675 "parser.tab.c"
    break;

  case 54: /* expression: expression DIV expression  */
#line 304 "parser.y"
                                   { (yyval.node) = new BinaryOpNode("/",(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1681 "parser.tab.c"
    break;

  case 55: /* expression: expression LT expression  */
#line 305 "parser.y"
                                   { (yyval.node) = new ComparisonNode("<" ,(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1687 "parser.tab.c"
    break;

  case 56: /* expression: expression GT expression  */
#line 306 "parser.y"
                                   { (yyval.node) = new ComparisonNode(">" ,(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1693 "parser.tab.c"
    break;

  case 57: /* expression: expression LE expression  */
#line 307 "parser.y"
                                   { (yyval.node) = new ComparisonNode("<=",(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1699 "parser.tab.c"
    break;

  case 58: /* expression: expression GE expression  */
#line 308 "parser.y"
                                   { (yyval.node) = new ComparisonNode(">=",(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1705 "parser.tab.c"
    break;

  case 59: /* expression: expression EQ expression  */
#line 309 "parser.y"
                                   { (yyval.node) = new ComparisonNode("==",(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1711 "parser.tab.c"
    break;

  case 60: /* expression: expression NEQ expression  */
#line 310 "parser.y"
                                   { (yyval.node) = new ComparisonNode("!=",(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1717 "parser.tab.c"
    break;

  case 61: /* expression: expression AND expression  */
#line 311 "parser.y"
                                   { (yyval.node) = new LogicalOpNode("&&",(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1723 "parser.tab.c"
    break;

  case 62: /* expression: expression OR expression  */
#line 312 "parser.y"
                                   { (yyval.node) = new LogicalOpNode("||",(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1729 "parser.tab.c"
    break;

  case 63: /* expression: NOT expression  */
#line 313 "parser.y"
                                   { (yyval.node) = new LogicalNotNode((yyvsp[0].node)); }
#line 1735 "parser.tab.c"
    break;

  case 64: /* expression: ID LPAREN arg_list RPAREN  */
#line 315 "parser.y"
        {
            (yyval.node) = new FunctionCallNode(std::string((yyvsp[-3].id)), *(yyvsp[-1].arglist));
            delete (yyvsp[-1].arglist);
        }
#line 1744 "parser.tab.c"
    break;

  case 65: /* expression: ID LBRACKET expression RBRACKET  */
#line 319 "parser.y"
                                      { (yyval.node) = new ArrayAccessNode((yyvsp[-3].id),(yyvsp[-1].node)); }
#line 1750 "parser.tab.c"
    break;

  case 66: /* expression: ID  */
#line 320 "parser.y"
                                      { (yyval.node) = new IdentifierNode((yyvsp[0].id)); }
#line 1756 "parser.tab.c"
    break;

  case 67: /* expression: NUMBER  */
#line 321 "parser.y"
                                      { (yyval.node) = new NumberNode((yyvsp[0].num)); }
#line 1762 "parser.tab.c"
    break;


#line 1766 "parser.tab.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

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
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
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
  ++yynerrs;

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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
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
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 324 "parser.y"


void yyerror(const char *s) {
    printf("Parse error: %s\n", s);
}
