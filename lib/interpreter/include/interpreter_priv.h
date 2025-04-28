#ifndef INTERPRETER_PRIV_H
#define INTERPRETER_PRIV_H

#include "interpreter.h"
#include "ast_priv.h"

bool _ast_expr_check  (ASTN_Expr*, HashTable*, Stack**, const char*);
void _error_underline (const char*, const uint32_t, const uint32_t, const uint32_t);

#endif // !INTERPRETER_PRIV_H
