#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "arena.h"

typedef struct ast        AST;
typedef struct astn_stmt  ASTN_Stmt;
typedef struct astn_expr  ASTN_Expr;
typedef struct astn_id    ASTN_Ident;
typedef struct astn_token ASTN_Token;

AST*        ast_create             (Arena, const char*, ASTN_Stmt*);

ASTN_Stmt*  astn_add_stmt          (ASTN_Stmt*, ASTN_Stmt*);
ASTN_Stmt*  astn_create_stmt       (Arena, ASTN_Ident*, ASTN_Expr*, const uint32_t, const uint32_t, const uint32_t, const uint32_t);

ASTN_Expr*  astn_create_expr_app   (Arena, ASTN_Expr*, ASTN_Expr*, const uint32_t, const uint32_t, const uint32_t, const uint32_t);
ASTN_Expr*  astn_create_expr_abs   (Arena, ASTN_Ident*, ASTN_Expr*, const uint32_t, const uint32_t, const uint32_t, const uint32_t);
ASTN_Expr*  astn_create_expr_ident (Arena, ASTN_Ident*);

ASTN_Ident* astn_create_ident      (Arena, ASTN_Token*, ASTN_Ident*);

ASTN_Token* astn_create_token      (Arena, const char*, const uint32_t, const uint32_t, const uint32_t);

ASTN_Expr*  astn_copy_expr         (ASTN_Expr*);
ASTN_Ident* astn_copy_ident        (ASTN_Ident*);
ASTN_Token* astn_copy_token        (ASTN_Token*);

void        astn_free_expr         (ASTN_Expr*);
void        astn_free_ident        (ASTN_Ident*);
void        astn_free_token        (ASTN_Token*);

#endif // !AST_H
