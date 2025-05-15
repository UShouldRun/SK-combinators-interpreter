#include "ast_priv.h"

// ========================# PUBLIC #========================

AST* ast_create(Arena arena, const char* filename, ASTN_Stmt* stmts) {
  assert(arena != NULL);
  assert(filename != NULL);

  if (stmts == NULL)
    return NULL;

  AST* ast = (AST*)arena_alloc(arena, sizeof(struct ast));
  assert(ast != NULL);

  size_t s_stmts = 0;
  ASTN_Stmt* stmt = stmts;
  for (; stmt != NULL; stmt = stmt->next, s_stmts++);

  *ast = (AST){
    .filename = filename,
    .s_stmts = s_stmts,
    .stmts = stmts
  };
  return ast;
}

ASTN_Stmt* astn_add_stmt(ASTN_Stmt* head, ASTN_Stmt* next) {
  assert(head != NULL && head->next == NULL);
  head->next = next;
  return head;
}

ASTN_Stmt* astn_create_stmt(
  Arena arena, ASTN_Ident* var, ASTN_Expr* expr,
  const uint32_t frow, const uint32_t fcol, const uint32_t erow, const uint32_t ecol
) {
  assert(arena != NULL);
  if (var == NULL || expr == NULL)
    return NULL;

  ASTN_Stmt* stmt = (ASTN_Stmt*)arena_alloc(arena, sizeof(struct astn_stmt));
  assert(stmt != NULL);

  *stmt = (ASTN_Stmt){
    .frow    = frow,
    .fcol    = fcol,
    .erow    = erow,
    .ecol    = ecol,
    .var     = var,
    .expr    = expr,
    .sk_expr = NULL,
    .next    = NULL
  };
  return stmt;
}

ASTN_Expr* astn_create_expr_app(
  Arena arena, ASTN_Expr* left, ASTN_Expr* right,
  const uint32_t frow, const uint32_t fcol, const uint32_t erow, const uint32_t ecol
) {
  assert(arena != NULL);
  assert(left != NULL);
  assert(right != NULL);
  
  ASTN_Expr* expr = (ASTN_Expr*)arena_alloc(arena, sizeof(struct astn_expr));
  assert(expr != NULL);

  *expr = (ASTN_Expr){
    .frow             = frow,
    .fcol             = fcol,
    .erow             = erow,
    .ecol             = ecol,
    .type             = EXPR_APP,
    .fields.app.left  = left,
    .fields.app.right = right
  };
  return expr;
}

ASTN_Expr* astn_create_expr_abs(
  Arena arena, ASTN_Ident* vars, ASTN_Expr* sub_expr,
  const uint32_t frow, const uint32_t fcol, const uint32_t erow, const uint32_t ecol
) {
  assert(arena != NULL);
  assert(vars != NULL);
  assert(sub_expr != NULL);

  ASTN_Expr* expr = (ASTN_Expr*)arena_alloc(arena, sizeof(struct astn_expr));
  assert(expr != NULL);

  *expr = (ASTN_Expr){
    .frow            = frow,
    .fcol            = fcol,
    .erow            = erow,
    .ecol            = ecol,
    .type            = EXPR_ABS,
    .fields.abs.vars = vars,
    .fields.abs.expr = sub_expr 
  };
  return expr;
}

ASTN_Expr* astn_create_expr_ident(Arena arena, ASTN_Ident* var) {
  assert(arena != NULL);
  assert(var != NULL);

  ASTN_Expr* expr = (ASTN_Expr*)arena_alloc(arena, sizeof(struct astn_expr));
  assert(expr != NULL);

 *expr = (ASTN_Expr){
    .frow       = var->frow,
    .fcol       = var->fcol,
    .erow       = var->erow,
    .ecol       = var->ecol,
    .type       = EXPR_IDENT,
    .fields.var = var,
  };
  return expr;
}

ASTN_Ident* astn_create_ident(Arena arena, ASTN_Token* token, ASTN_Ident* next) {
  assert(arena != NULL);
  assert(token != NULL);

  ASTN_Ident* id = (ASTN_Ident*)arena_alloc(arena, sizeof(struct astn_id));
  assert(id != NULL);

  *id = (ASTN_Ident){
    .frow  = token->frow,
    .fcol  = token->fcol,
    .erow  = next != NULL ? next->erow : token->frow,
    .ecol  = next != NULL ? next->ecol : token->ecol,
    .token = token,
    .next  = next
  };
  return id;
}

ASTN_Token* astn_create_token(Arena arena, const char* str, const uint32_t frow, const uint32_t fcol, const uint32_t ecol) {
  assert(arena != NULL);
  assert(str != NULL);

  ASTN_Token* token = (ASTN_Token*)arena_alloc(arena, sizeof(struct astn_token));
  assert(token != NULL);

  *token = (ASTN_Token){
    .frow = frow,
    .fcol = fcol,
    .ecol = ecol,
    .str  = str,
  };
  return token;
}
