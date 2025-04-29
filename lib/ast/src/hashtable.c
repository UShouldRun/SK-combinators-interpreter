#include "hashtable_priv.h"

// ===================# PUBLIC #=======================

HashTable hashtable_create(size_t s_hashtable, float load_treshold_factor) {
  return (HashTable)hashmap_create(s_hashtable, load_treshold_factor);
}

size_t hashtable_size(HashTable table) {
  return hashmap_size(table);
}

bool hashtable_exists(HashTable table, ASTN_Token* token) {
  return hashmap_exists(table, (char*)token->str);
}

bool hashtable_insert(HashTable* table, ASTN_Stmt* stmt) {
  return hashmap_insert(table, (char*)stmt->var->token->str, (void*)stmt, NULL);
}

bool hashtable_free(HashTable table) {
  return hashmap_free(table, NULL, false);
}

ASTN_Stmt* hashtable_lookup(HashTable table, ASTN_Token* token) {
  return (ASTN_Stmt*)hashmap_get(table, (char*)token->str);
}

ASTN_Stmt* hashtable_remove(HashTable table, ASTN_Token* token) {
  ASTN_Stmt* stmt = (ASTN_Stmt*)hashmap_get(table, (char*)token->str);
  (void)hashmap_remove(table, (char*)token->str, NULL, false);
  return stmt;
}

Stack* stack_create() {
  size_t s_stack = 20;

  Stack* stack = (Stack*)malloc(sizeof(struct stack) + s_stack * sizeof(struct astn_token*));
  assert(stack != NULL);

  *stack = (Stack){
    .s_stack = s_stack,
    .top     = -1
  };
  return stack;
}

size_t stack_size(Stack* stack) {
  return stack != NULL ? stack->s_stack : 0;
}

ASTN_Token* stack_pop(Stack** stack) {
  if (stack == NULL || *stack == NULL || (*stack)->top == -1)
    return NULL;

  Stack* _stack = *stack;
  ASTN_Token* token = _stack->array[_stack->top];
  _stack->array[_stack->top] = NULL;
  _stack->top--;

  if (_stack->top + 10 < (int64_t)_stack->s_stack / 5) {
    size_t new_size = _stack->s_stack / 2;
    Stack* temp = (Stack*)realloc(_stack, sizeof(struct stack) + new_size * sizeof(struct astn_token*));
    assert(temp != NULL);
    temp->top     = _stack->top;
    temp->s_stack = _stack->s_stack;
    *stack = temp;
  }

  return token;
}

bool stack_push(Stack** stack, ASTN_Token* token) {
  if (stack == NULL || *stack == NULL)
    return false;

  Stack* _stack = *stack;
  if (_stack->top >= (int64_t)_stack->s_stack - 1) {
    size_t new_size = 2 * _stack->s_stack;

    Stack* temp = (Stack*)realloc(_stack, sizeof(struct stack) + new_size * sizeof(struct astn_token*));
    assert(temp != NULL);

    temp->top     = _stack->top;
    temp->s_stack = _stack->s_stack;
    *stack = temp;
    _stack = temp;
  }

  (*stack)->array[++((*stack)->top)] = token;
  return true;
}

bool stack_exists(Stack* stack, ASTN_Token* token) {
  if (stack == NULL || token == NULL)
    return false;

  int64_t i = 0;
  ASTN_Token* curr = stack->array[stack->top];
  while (stack->top - i >= 0) {
    if (strcmp(token->str, curr->str) == 0)
      return true;
    curr = stack->array[stack->top - ++i];
  }
  return false;
}

bool stack_clear(Stack* stack) {
  if (stack == NULL)
    return false;
  int64_t top = stack->top;
  for (int64_t i = 0; i <= top; stack->array[i++] = NULL);
  stack->top = -1;
  return true;
}

bool stack_free(Stack* stack) {
  if (stack == NULL)
    return false;
  free(stack);
  return true;
}
