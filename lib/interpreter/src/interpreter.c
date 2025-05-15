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

SK_Tree* ast_convert(AST* ast, HashTable table, const char* filename) {
  assert(ast != NULL && table != NULL);

  size_t s_stmts = ast->s_stmts;
  SK_Tree* roots = (SK_Tree*)malloc(s_stmts * sizeof(struct sk_tree));
  assert(roots != NULL);

  for (size_t i = 0; i < s_stmts; i++)
    roots[i] = _ast_expr_convert(ast->stmts[i].expr, table, ast->filename);

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

SK_Tree* _ast_expr_convert(ASTN_Expr* expr, HashTable table, const char* filename) {
  assert(expr != NULL && table != NULL);

  switch (expr->type) {
    case EXPR_APP: {
      SK_Tree* app = (SK_Tree*)malloc(sizeof(struct sk_tree));
      assert(app != NULL);

      *app = (SK_Tree){
        .type  = APP_NODE,
        .left  = _ast_expr_convert(expr->fields.app.left, table, filename),
        .right = _ast_expr_convert(expr->fields.app.right, table, filename)
      };

      return app;
    }
    case EXPR_ABS: {
      ASTN_Ident* var = expr->fields.abs.vars;

      if (expr->fields.abs.expr->type != EXPR_APP) {
        if (expr->fields.abs.expr->type == EXPR_IDENT) {
          if (strcmp(expr->fields.abs->expr->fields.var->token->str, var->token->str) == 0) {
            SK_Tree* app1, app2, s, k1, k2;
            app1 = (SK_Tree*)malloc(sizeof(struct sk_tree));
            app2 = (SK_Tree*)malloc(sizeof(struct sk_tree));
            s    = (SK_Tree*)malloc(sizeof(struct sk_tree));
            k1   = (SK_Tree*)malloc(sizeof(struct sk_tree));
            k2   = (SK_Tree*)malloc(sizeof(struct sk_tree));
            assert(app1 != NULL && app2 != NULL && s != NULL && k1 != NULL && k2 != NULL);

            *s  = (SK_Tree){ .type = S_NODE, .left = .right = NULL };
            *k2 = (SK_Tree){ .type = K_NODE, .left = .right = NULL };
            *k1 = (SK_Tree){ .type = K_NODE, .left = .right = NULL };

            *app2 = (SK_Tree){ .type = APP_NODE, .left = s, .right = k2 };
            *app1 = (SK_Tree){ .type = APP_NODE, .left = app2, .right = k1 };

            return app1;
          }

          ASTN_Stmt* stmt = hashtable_lookup(table, var->token);
          assert(stmt != NULL);

          if (stmt->sk_expr == NULL) {
            fprintf(
              stderr,
              "[SK CONVERTER]: sub expression was not defined previously to the current statement %s at line %d in file %s. Maybe you declared it later?\n",
              var->token->str,
              var->frow,
              filename
            );
            _error_underline(filename, var->frow, var->fcol, var->ecol);
          }

          SK_Tree* ref = (SK_Tree*)malloc(sizeof(struct sk_tree));
          assert(ref != NULL);
          *ref = (SK_Tree){ .type = REF_NODE, .left = stmt->sk_tree, .right = NULL };

          return ref;
        }

        if (!_ast_in_free_var_set(expr->fields.abs.expr, var)) {
          SK_Tree* k = (SK_Tree*)malloc(sizeof(struct sk_tree));
          assert(k != NULL);
          *k = (SK_Tree){
            .type = K_NODE,
            .left = .right = NULL,
          };

          SK_Tree* app = (SK_Tree*)malloc(sizeof(struct sk_tree));
          assert(app != NULL);
          *app = (SK_Tree){
            .type  = APP_NODE,
            .left  = k,
            .right = _ast_expr_convert(expr->fields.abs.expr, table, filename);
          };

          return app;
        }

        SK_Tree* app1 = (SK_Tree*)malloc(sizeof(struct sk_tree));
        assert(app1 != NULL);

        SK_Tree* app2 = (SK_Tree*)malloc(sizeof(struct sk_tree));
        assert(app2 != NULL);

        SK_Tree* s = (SK_Tree*)malloc(sizeof(struct sk_tree));
        assert(s != NULL);

        bool var_free_left  = _ast_in_free_var_set(expr->fields.abs.expr->fields.app.left, var);
        bool var_free_right = _ast_in_free_var_set(expr->fields.abs.expr->fields.app.right, var);

        if (!var_free_left) {
          SK_Tree* k = (SK_Tree*)malloc(sizeof(struct sk_tree));
          assert(k != NULL);
          *k = (SK_Tree){
            .type = K_NODE,
            .left = .right = NULL,
          };

          SK_Tree* app3 = (SK_Tree*)malloc(sizeof(struct sk_tree));
          assert(app3 != NULL);
          *app3 = (SK_Tree){
            .type  = APP_NODE,
            .left  = k,
            .right = _ast_expr_convert(expr->fields.abs.expr->fiels.app.left, table, filename);
          };

          *app2 = (SK_Tree){
            .type  = APP_NODE,
            .left  = s,
            .right = app3
          };
        }

        // Continue from HERE. NOTE: I'LL PROBABLY NEED TO INTRODUCE A ASTN_Ident* CONTEXT FOR FREE VARIABLE CONVERTION
        // THIS CASE: S (\T x -> P) (\T x -> Q)
      }
    }
  }
}

bool _ast_in_free_var_set(ASTN_Expr* expr, ASTN_Ident* var) {
  if (expr == NULL || var == NULL)
    return false;

  switch (expr->type) {
    case EXPR_APP: { 
      return (
           _ast_in_free_var_set(expr->fields.app.left, var)
        || _ast_in_free_var_set(expr->fields.app.right, var)
      );
    }
    case EXPR_ABS: {
      return _ast_in_free_var_set(expr->fields.abs.expr, var);
    }
    case EXPR_IDENT: {
      return strcmp(expr->fields.var->token->str, var->token->str) == 0;
    }
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
