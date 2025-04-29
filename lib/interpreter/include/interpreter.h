#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "hashtable.h"

HashTable ast_check (AST*, size_t);
void      ast_print (AST*);

#endif // !INTERPRETER_H
