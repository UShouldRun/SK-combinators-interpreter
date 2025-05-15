#ifndef AST_PRIV_H
#define AST_PRIV_H

#include "hashtable.h"

struct sk_tree {
  enum { S_NODE, K_NODE, APP_NODE, REF_NODE } type;
  struct sk_tree* left, right;
};

struct ast {
  size_t      s_stmts;
  ASTN_Stmt*  stmts;
  const char* filename;
};

struct astn_stmt {
  uint32_t frow, fcol, erow, ecol;
  ASTN_Ident* var;
  ASTN_Expr*  expr;
  ASTN_Stmt*  next;
  struct sk_tree* sk_expr;
};

struct astn_expr {
  uint32_t frow, fcol, erow, ecol;
  enum { EXPR_APP, EXPR_ABS, EXPR_IDENT } type;
  union {
    struct {
      ASTN_Expr* left, *right;
    } app;
    struct {
      ASTN_Ident* vars;
      ASTN_Expr*  expr;
    } abs;
    ASTN_Ident* var;
  } fields;
};

struct astn_id {
  uint32_t frow, fcol, erow, ecol;
  size_t      s_id;
  ASTN_Token* token;
  ASTN_Ident* next;
};

struct astn_token {
  uint32_t frow, fcol, ecol;
  const char* str;
};

#define MAX_STRUCT(a, b) sizeof(a) > sizeof(b) ? sizeof(a) : sizeof(b)
#define MAX_SIZE MAX_STRUCT( \
  struct astn_expr, \
  MAX_STRUCT( \
    struct astn_stmt, \
    MAX_STRUCT( \
      struct ast, \
      MAX_STRUCT( \
        struct astn_id, \
        struct astn_token \
      ) \
    ) \
  ) \
)

#endif // !AST_PRIV_H
