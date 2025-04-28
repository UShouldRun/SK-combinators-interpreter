#ifndef HASHTABLE_PRIV_H
#define HASHTABLE_PRIV_H

#include "hashtable.h"
#include "ast_priv.h"

struct stack {
  size_t s_stack, top;
  ASTN_Token* array[];
};

#endif // !HASHTABLE_PRIV_H
