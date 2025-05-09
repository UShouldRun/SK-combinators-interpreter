#include "interpreter_priv.h"

// ========================# PUBLIC #========================

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

  stack_free(stack);
  return table;
}

void ast_print(AST* ast) {
  assert(ast != NULL);
  assert(ast->stmts != NULL);

  IdentList* list = NULL;
  for (ASTN_Stmt* stmt = ast->stmts; stmt != NULL; stmt = stmt->next) {
    fprintf(stdout, "%s\n", stmt->var->token->str);
    list = _ident_list_append(list, true);
    _ast_expr_print(stmt->expr, 0, list);
    _ident_list_free(list);
    list = NULL;
  }
}

void ast_transform(Arena arena, AST* ast) {
  assert(arena != NULL && ast != NULL);
  for (ASTN_Stmt* stmt = ast->stmts; stmt != NULL; stmt = stmt->next)
    _ast_expr_transform(arena, stmt->expr);
}

SK_Tree* ast_convert(AST* ast, HashTable table) {
  assert(ast != NULL && table != NULL);

  size_t s_stmts = ast->s_stmts;
  SK_Tree* roots = (SK_Tree*)malloc(s_stmts * sizeof(struct sk_tree));
  assert(roots != NULL);

  for (size_t i = 0; i < s_stmts; i++)
    roots[i] = _ast_convert_expr(ast->stmts[i], table);

  return roots;
}

SK_Tree* skt_beta_redu(SK_Tree* root) {
  return NULL;
}

void skt_print(SK_Tree* root) {

}

// ========================# PRIVATE #========================

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
          "[CHECKER]: non declared identifier used %s in file %s at %u\n",
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

void _ast_expr_print(ASTN_Expr* expr, size_t depth, IdentList* list) {
  assert(expr != NULL);
  
  IdentList* node = list;
  for (size_t i = 0; i < depth; i++, node = node->next)
    printf("%s   ", node->value ? " " : "│");
  
  printf("%s ", node->value ? "└──" : "├──");
  
  switch (expr->type) {
    case EXPR_APP: {
      printf("@\n");
      
      list = _ident_list_append(list, false);
      _ast_expr_print(expr->fields.app.left, depth + 1, list);
      (void)_ident_list_remove(&list);
      
      list = _ident_list_append(list, true);
      _ast_expr_print(expr->fields.app.right, depth + 1, list);
      (void)_ident_list_remove(&list);
      
      break;
    }
    case EXPR_ABS: {
      printf("λ");
      
      ASTN_Ident* var = expr->fields.abs.vars;
      while (var != NULL) {
        printf("%s", var->token->str);
        var = var->next;
        if (var != NULL)
          printf(",");
      }
      printf("\n");
      
      list = _ident_list_append(list, true);
      _ast_expr_print(expr->fields.abs.expr, depth + 1, list);
      (void)_ident_list_remove(&list);
      
      break;
    }
    case EXPR_IDENT: {
      printf("%s\n", expr->fields.var->token->str);
      break;
    }
  }
}

void _ast_expr_transform(Arena arena, ASTN_Expr* expr) {
  assert(arena != NULL && expr != NULL);  

  switch (expr->type) {
    case EXPR_APP: {
      _ast_expr_transform(arena, expr->fields.app.left);
      _ast_expr_transform(arena, expr->fields.app.right);
      break;
    }
    case EXPR_ABS: {
      ASTN_Ident* var = expr->fields.abs.vars->next;
      expr->fields.abs.vars->next = NULL;

      while (var != NULL) {
        ASTN_Ident* temp = var->next;
        var->next = NULL;

        expr->fields.abs.expr = astn_create_expr_abs(
          arena, var, expr->fields.abs.expr,
          var->frow, var->fcol, var->erow, var->ecol
        );
        
        var = temp;
      }

      _ast_expr_transform(arena, expr->fields.abs.expr);
      break;
    }
    case EXPR_IDENT: {}
  }
}

IdentList* _ident_list_append(IdentList* list, bool value) {
  IdentList* node = (IdentList*)malloc(sizeof(struct ident_list));
  assert(node != NULL);
  node->value = value;
  node->next  = NULL;

  if (list == NULL)
    return node;

  IdentList* n = list;
  for (; n->next != NULL; n = n->next);
  n->next = node;

  return list;
}

bool _ident_list_remove(IdentList** list) {
  assert(list != NULL);

  IdentList** node = list;
  for (; (*node)->next != NULL; node = &((*node)->next));

  bool value = (*node)->value;
  IdentList* delete = *node;

  *node = NULL;
  free(delete);

  return value;
}

void _ident_list_free(IdentList* list) {
  while (list != NULL) {
    IdentList* delete = list;
    list = list->next;
    free(delete);
  }
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
  for (uint32_t i = 1; i <= s_word; i++)
    underline[i] = '~';
  underline[s_word] = '\0';
  fprintf(stderr, "%s%s\n", underline, "\033[0m");
}
