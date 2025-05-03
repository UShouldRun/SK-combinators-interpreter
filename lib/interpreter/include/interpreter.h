#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "hashtable.h"

// ========================# PUBLIC #========================

typedef struct sk_tree SK_Tree;

HashTable ast_check     (AST*, size_t);
void      ast_print     (AST*);
void      ast_transform (Arena, AST*);
SK_Tree*  ast_convert   (AST*, HashTable);
SK_Tree*  skt_beta_redu (SK_Tree*);
void      skt_print     (SK_Tree*);

#endif // !INTERPRETER_H
