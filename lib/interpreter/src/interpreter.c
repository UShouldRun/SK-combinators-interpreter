#include "interpreter_priv.h"

HashTable ast_check(AST* ast, size_t s_hashtable) {
  if (ast == NULL)
    return NULL;

  HashTable table = hashtable_create(s_hashtable, .75);
  Stack* stack = stack_create();
  for (ASTN_Stmt* stmt = ast->stmts; stmt != NULL; stmt = stmt->next) {
    if (hashtable_exists(table, stmt->var->token)) {
      fprintf(
        stderr,
        "[CHECKER]: reassigning expression to const variable %s in file %s at %u\n",
        stmt->var->token->str, ast->filename, stmt->frow
      );
      _error_underline(ast->filename, stmt->frow, stmt->var->fcol, stmt->var->ecol);
    }

    hashtable_insert(&table, stmt);
    _ast_expr_check(stmt->expr, &table, &stack, ast->filename);
    stack_clear(stack);
  }

  return table;
}


bool _ast_expr_check(ASTN_Expr* expr, HashTable* table, Stack** stack, const char* filename) {
  if (expr == NULL || table == NULL || stack == NULL)
    return false;

  switch (expr->type) {
    case EXPR_IDENT: {
      ASTN_Token* token = expr->fields.var->token;
      const bool check = stack_exists(*stack, token) || hashtable_exists(*table, token);
      if (!check) {
         fprintf(
          stderr,
          "[CHECKER]: non declared identfier used %s in file %s at %u\n",
          token->str, filename, token->frow
        );
        _error_underline(filename, token->frow, token->fcol, token->ecol);
      }
      return check;
    }
    case EXPR_ABS: {
      for (ASTN_Ident* var = expr->fields.abs.vars; var != NULL; var = var->next)
        (void)stack_push(stack, var->token);
      const bool check = _ast_expr_check(expr->fields.abs.expr, table, stack, filename);
      for (ASTN_Ident* var = expr->fields.abs.vars; var != NULL; var = var->next)
        (void)stack_pop(stack);
      return check;
    }
    case EXPR_APP: {
      ASTN_Expr* sub_expr = expr->fields.app.left;
      if (sub_expr->type == EXPR_ABS)
        (void)stack_push(stack, NULL);

      const bool check_left = _ast_expr_check(sub_expr, table, stack, filename);
      if (sub_expr->type == EXPR_ABS && expr->fields.app.right->type != EXPR_ABS)
        (void)stack_pop(stack);

      sub_expr = expr->fields.app.right;
      const bool check_right = _ast_expr_check(sub_expr, table, stack, filename);
      if (sub_expr->type == EXPR_ABS)
        (void)stack_pop(stack);

      return check_left && check_right;
    }
  }

  return false;
}

void _error_underline(const char* filename, const uint32_t frow, const uint32_t fcol, const uint32_t ecol) {
  assert(filename != NULL);

  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "Error: Could not access or find the source file: %s\n", filename);
    return;
  }

  char line[1024];
  for (
    uint32_t current_line = 1;
    fgets(line, sizeof(line), file) && current_line != frow;
    current_line++
  );
  fclose(file);

  fprintf(stderr, "%s", line);

  uint32_t s_word = ecol - fcol + 1;
  char offset[fcol], underline[s_word + 1];

  for (uint32_t i = 0; i < fcol - 1; i++)
    offset[i] = ' ';
  offset[fcol - 1] = '\0';
  fprintf(stderr, "%s%s", offset, "\033[31m");

  underline[0] = '^';
  for (uint32_t i = 1; i < s_word; i++)
    underline[i] = '~';
  underline[s_word] = '\0';
  fprintf(stderr, "%s%s\n", underline, "\033[0m");
}
