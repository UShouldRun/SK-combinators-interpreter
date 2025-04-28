#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <string.h>
#include "hashmap.h"
#include "ast.h"

typedef HashMap      HashTable;
typedef struct stack Stack;

HashTable   hashtable_create (size_t, float);
size_t      hashtable_size   (HashTable);
bool        hashtable_exists (HashTable, ASTN_Token*);
bool        hashtable_insert (HashTable*, ASTN_Stmt*);
bool        hashtable_free   (HashTable);
ASTN_Stmt*  hashtable_lookup (HashTable, ASTN_Token*);
ASTN_Stmt*  hashtable_remove (HashTable, ASTN_Token*);

Stack*      stack_create     ();
size_t      stack_size       (Stack*);
ASTN_Token* stack_pop        (Stack**);
bool        stack_push       (Stack**, ASTN_Token*);
bool        stack_exists     (Stack*, ASTN_Token*); // in current frame
bool        stack_clear      (Stack*);
bool        stack_free       (Stack*);

#endif // !HASHTABLE_H
