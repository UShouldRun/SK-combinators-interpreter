#ifndef YY_PARSER_TAB_H_INCLUDED
#define YY_PARSER_TAB_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "arena.h"
#include "ast.h"

#ifndef YYTOKENTYPE
#define YYTOKENTYPE

enum yytokentype {
  TT_LET        = 258,
  TT_IDENTIFIER = 259,
  TT_ASSIGN     = 260,
  TT_LAMBDA     = 261,
  TT_COMMA      = 262,
  TT_ARROW      = 263,
  TT_LPAREN     = 264,
  TT_RPAREN     = 265,
  TT_SEMI       = 266,

  // TT_LIT_NUMBER = 258,
  // TT_LIT_REAL   = 259,
  // TT_LIT_STR    = 260
};

#endif // !YYTOKENTYPE

#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE {
  int32_t first_line, first_column, last_line, last_column;
} YYLTYPE;
#define YYLTYPE_IS_DECLARED 1
#endif

extern YYLTYPE yylloc;
int32_t yyparse(void);

#endif /* !YY_PARSER_TAB_H_INCLUDED */
