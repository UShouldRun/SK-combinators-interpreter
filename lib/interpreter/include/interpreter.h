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
SK_Tree** ast_convert   (Arena, AST*, HashTable);
SK_Tree*  skt_beta_redu (Arena, SK_Tree*);
SK_Tree*  skt_copy      (Arena, SK_Tree*);
void      skt_print     (SK_Tree**, size_t);

#endif // !INTERPRETER_H
