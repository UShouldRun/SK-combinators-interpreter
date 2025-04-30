#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "hashtable.h"

// ========================# PUBLIC #========================

HashTable ast_check     (AST*, size_t);
void      ast_print     (AST*);
void      ast_transform (Arena, AST*);

#endif // !INTERPRETER_H
