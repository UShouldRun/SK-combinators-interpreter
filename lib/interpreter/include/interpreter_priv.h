#ifndef INTERPRETER_PRIV_H
#define INTERPRETER_PRIV_H

#include "interpreter.h"
#include "ast_priv.h"

// ========================# PRIVATE #========================

typedef struct ident_list {
  bool value;
  struct ident_list* next;
} IdentList;

IdentList* _ident_list_append   (IdentList*, bool);
bool       _ident_list_remove   (IdentList**);
void       _ident_list_free     (IdentList*);

bool        _ast_expr_check     (ASTN_Expr*, HashTable*, Stack**, const char*);
void        _ast_expr_print     (ASTN_Expr*, size_t, IdentList*);
void        _ast_expr_transform (Arena, ASTN_Expr*);
void        _error_underline    (const char*, const uint32_t, const uint32_t, const uint32_t);

#endif // !INTERPRETER_PRIV_H
