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

SK_Tree** ast_convert(Arena arena, AST* ast, HashTable table) {
  assert(arena != NULL && ast != NULL && table != NULL);

  size_t s_stmts = ast->s_stmts;
  SK_Tree** roots = (SK_Tree**)arena_alloc(arena, s_stmts * sizeof(SK_Tree*));
  assert(roots != NULL);

  ASTN_Stmt* stmt = ast->stmts;
  for (size_t i = 0; i < s_stmts; i++, stmt = stmt->next) {
    roots[i] = _ast_expr_convert(arena, stmt->expr, table, ast->filename);
    roots[i]->ld_ident = stmt->var;
    stmt->sk_expr = roots[i];
  }

  return roots;
}

SK_Tree* skt_beta_redu(SK_Tree* root) {
  if (root == NULL)
    return NULL;
  return root;
}

void skt_print(SK_Tree** roots, size_t s_roots) {
  assert(roots != NULL);

  IdentList* list = NULL;
  for (size_t i = 0; i < s_roots; i++) {
    fprintf(stdout, "%s\n", roots[i]->ld_ident->token->str);
    list = _ident_list_append(list, true);
    _sk_print_expr(roots[i], 0, list);
    _ident_list_free(list);
    list = NULL;
  }
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
      ASTN_Ident* var = expr->fields.abs.vars;
      ASTN_Expr** sub_expr = &(expr->fields.abs.expr);

      while (var->next != NULL) {
        ASTN_Ident* next = var->next;
        var->next = NULL;

        *sub_expr = astn_create_expr_abs(
          arena, next, *sub_expr,
          var->frow, var->fcol, var->erow, var->ecol
        );
        
        var = next;
        sub_expr = &((*sub_expr)->fields.abs.expr);
      }

      _ast_expr_transform(arena, expr->fields.abs.expr);
      break;
    }
    case EXPR_IDENT: {}
  }
}

SK_Tree* _ast_expr_convert(Arena arena, ASTN_Expr* expr, HashTable table, const char* filename) {
  assert(arena != NULL && expr != NULL && table != NULL);

  switch (expr->type) {
    case EXPR_APP: {
      SK_Tree* app = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
      assert(app != NULL);

      *app = (SK_Tree){
        .type  = APP_NODE,
        .left  = _ast_expr_convert(arena, expr->fields.app.left, table, filename),
        .right = _ast_expr_convert(arena, expr->fields.app.right, table, filename)
      };

      return app;
    }
    case EXPR_ABS: {
      ASTN_Ident* var = expr->fields.abs.vars;

      if (expr->fields.abs.expr->type != EXPR_APP) {
        if (expr->fields.abs.expr->type == EXPR_IDENT) {
          if (strcmp(expr->fields.abs.expr->fields.var->token->str, var->token->str) == 0) {
            SK_Tree* app1, *app2, *s, *k1, *k2;
            app1 = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
            app2 = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
            s    = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
            k1   = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
            k2   = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
            assert(app1 != NULL && app2 != NULL && s != NULL && k1 != NULL && k2 != NULL);

            *s  = (SK_Tree){ .type = S_NODE, .left = NULL, .right = NULL, .ld_ident = NULL };
            *k2 = (SK_Tree){ .type = K_NODE, .left = NULL, .right = NULL, .ld_ident = NULL };
            *k1 = (SK_Tree){ .type = K_NODE, .left = NULL, .right = NULL, .ld_ident = NULL };

            *app2 = (SK_Tree){ .type = APP_NODE, .left = s, .right = k2, .ld_ident = NULL };
            *app1 = (SK_Tree){ .type = APP_NODE, .left = app2, .right = k1, .ld_ident = NULL };

            return app1;
          }

          SK_Tree* k = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
          assert(k != NULL);

          ASTN_Stmt* stmt = hashtable_lookup(table, var->token);
          if (stmt != NULL) {
            if (stmt->sk_expr == NULL) {
              fprintf(
                stderr,
                "[SK CONVERTER]: sub expression was not defined previously to the current statement %s at line %d in file %s. Maybe you declared it later?\n",
                expr->fields.var->token->str,
                expr->fields.var->frow,
                filename
              );
              _error_underline(filename, expr->fields.var->frow, expr->fields.var->fcol, expr->fields.var->ecol);
            }

            SK_Tree* app = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
            assert(app != NULL);

            SK_Tree* ref = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
            assert(ref != NULL);
            *ref = (SK_Tree){ .type = REF_NODE, .left = stmt->sk_expr, .right = NULL, .ld_ident = stmt->var };

            *app = (SK_Tree){ .type = APP_NODE, .left = k, .right = ref, .ld_ident = NULL };

            return app;
          }

          *k = (SK_Tree){ .type = K_NODE, .left = NULL, .right = NULL, .ld_ident = NULL };
          return k;
        }

        SK_Tree* sub_expr = _ast_expr_convert(arena, expr->fields.abs.expr, table, filename);
        if (!_ast_in_free_var_set(expr->fields.abs.expr, var)) {
          SK_Tree* app = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
          assert(app != NULL);

          SK_Tree* k = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
          assert(k != NULL);

          *k = (SK_Tree){ .type = K_NODE, .left = NULL, .right = NULL, .ld_ident = NULL };

          *app = (SK_Tree){
            .type  = APP_NODE,
            .left  = k,
            .right = sub_expr,
            .ld_ident = NULL
          };

          return app;
        }

        return _ast_expr_convert_sk(arena, sub_expr, expr->fields.abs.vars);
      }

      bool var_free_left  = _ast_in_free_var_set(expr->fields.abs.expr->fields.app.left, var);
      bool var_free_right = _ast_in_free_var_set(expr->fields.abs.expr->fields.app.right, var);

      if (!var_free_left && !var_free_right) {
        SK_Tree* k = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
        assert(k != NULL);
        *k = (SK_Tree){
          .type  = K_NODE,
          .left  = NULL,
          .right = NULL,
          .ld_ident = NULL
        };

        SK_Tree* app = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
        assert(app != NULL);
        *app = (SK_Tree){
          .type  = APP_NODE,
          .left  = k,
          .right = _ast_expr_convert(arena, expr->fields.abs.expr, table, filename),
          .ld_ident = NULL
        };

        return app;
      }

      if (var_free_right && expr->fields.abs.expr->fields.app.right->type == EXPR_IDENT)
        return _ast_expr_convert(arena, expr->fields.abs.expr->fields.app.left, table, filename);

      SK_Tree* app1 = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
      assert(app1 != NULL);

      SK_Tree* app2 = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
      assert(app2 != NULL);

      SK_Tree* s = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
      assert(s != NULL);

      if (!var_free_left) {
        SK_Tree* k = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
        assert(k != NULL);
        *k = (SK_Tree){
          .type  = K_NODE,
          .left  = NULL,
          .right = NULL,
          .ld_ident = NULL
        };

        SK_Tree* app3 = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
        assert(app3 != NULL);
        *app3 = (SK_Tree){
          .type  = APP_NODE,
          .left  = k,
          .right = _ast_expr_convert(arena, expr->fields.abs.expr->fields.app.left, table, filename),
          .ld_ident = NULL
        };

        *app2 = (SK_Tree){
          .type  = APP_NODE,
          .left  = s,
          .right = app3,
          .ld_ident = NULL
        };
      } else {
        ASTN_Expr* sub_expr = astn_copy_expr(expr->fields.abs.expr->fields.app.left);
        assert(sub_expr != NULL);

        ASTN_Expr* left = (ASTN_Expr*)malloc(sizeof(struct astn_expr));
        assert(left != NULL);
        *left = (ASTN_Expr){
          .frow = expr->frow,
          .fcol = expr->fcol,
          .erow = expr->erow,
          .ecol = expr->ecol,
          .type = EXPR_ABS,
          .fields.abs.vars = astn_copy_ident(expr->fields.abs.vars),
          .fields.abs.expr = sub_expr
        };

        *app2 = (SK_Tree){
          .type  = APP_NODE,
          .left  = s,
          .right = _ast_expr_convert(arena, left, table, filename),
          .ld_ident = NULL
        };

        astn_free_expr(left);
      }

      if (!var_free_right) {
        SK_Tree* k = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
        assert(k != NULL);
        *k = (SK_Tree){
          .type  = K_NODE,
          .left  = NULL,
          .right = NULL,
          .ld_ident = NULL
        };

        SK_Tree* app3 = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
        assert(app3 != NULL);
        *app3 = (SK_Tree){
          .type  = APP_NODE,
          .left  = k,
          .right = _ast_expr_convert(arena, expr->fields.abs.expr->fields.app.right, table, filename),
          .ld_ident = NULL
        };

        *app1 = (SK_Tree){
          .type  = APP_NODE,
          .left  = app2,
          .right = app3,
          .ld_ident = NULL
        };
      } else {
        ASTN_Expr* sub_expr = astn_copy_expr(expr->fields.abs.expr->fields.app.right);
        assert(sub_expr != NULL);

        ASTN_Expr* right = (ASTN_Expr*)malloc(sizeof(struct astn_expr));
        assert(right != NULL);
        *right = (ASTN_Expr){
          .frow = expr->frow,
          .fcol = expr->fcol,
          .erow = expr->erow,
          .ecol = expr->ecol,
          .type = EXPR_ABS,
          .fields.abs.vars = astn_copy_ident(expr->fields.abs.vars),
          .fields.abs.expr = sub_expr
        };

        *app1 = (SK_Tree){
          .type  = APP_NODE,
          .left  = app2,
          .right = _ast_expr_convert(arena, right, table, filename),
          .ld_ident = NULL
        };

        astn_free_expr(right);
      }

      return app1;
    }
    case EXPR_IDENT: {
      ASTN_Stmt* stmt = hashtable_lookup(table, expr->fields.var->token);

      if (stmt != NULL) {
        if (stmt->sk_expr == NULL) {
          fprintf(
            stderr,
            "[SK CONVERTER]: sub expression was not defined previously to the current statement %s at line %d in file %s. Maybe you declared it later?\n",
            expr->fields.var->token->str,
            expr->fields.var->frow,
            filename
          );
          _error_underline(filename, expr->fields.var->frow, expr->fields.var->fcol, expr->fields.var->ecol);
        }

        SK_Tree* ref = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
        assert(ref != NULL);
        *ref = (SK_Tree){ .type = REF_NODE, .left = stmt->sk_expr, .right = NULL, .ld_ident = stmt->var };

        return ref;
      }

      SK_Tree* wrapper = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
      assert(wrapper != NULL);

      *wrapper = (SK_Tree){ .type = LD_NODE, .ld_ident = expr->fields.var, .left = NULL, .right = NULL };

      return wrapper;
    }
}

  fprintf(stderr, "[SK CONVERTER]: converter function reached its end. Something went wrong!\n");
  exit(1);
  return NULL;
}

SK_Tree* _ast_expr_convert_sk(Arena arena, SK_Tree* expr, ASTN_Ident* var) {
  assert(arena != NULL && expr != NULL && var != NULL);

  switch (expr->type) {
    case APP_NODE: {
      bool 
        var_free_left  = _ast_in_free_var_set_sk(expr->left, var),
        var_free_right = _ast_in_free_var_set_sk(expr->right, var)
      ;

      if (!var_free_left && var_free_right && expr->right->type == LD_NODE)
        return expr->left;

      SK_Tree* s = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
      assert(s != NULL);

      SK_Tree* app2 = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
      assert(app2 != NULL);

      SK_Tree* app1 = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
      assert(app1 != NULL);

      if (!var_free_left) {
        SK_Tree* k = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
        assert(k != NULL);

        SK_Tree* app3 = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
        assert(app3 != NULL);

        *k = (SK_Tree){ .type = K_NODE, .left = NULL, .right = NULL, .ld_ident = NULL };

        *app3 = (SK_Tree){
          .type  = APP_NODE,
          .left  = k,
          .right = expr->left,
          .ld_ident = NULL
        };

        *app2 = (SK_Tree){
          .type  = APP_NODE,
          .left  = s,
          .right = app3,
          .ld_ident = NULL
        };
      } else {
        *app2 = (SK_Tree){
          .type  = APP_NODE,
          .left  = s,
          .right = _ast_expr_convert_sk(arena, expr->left, var),
          .ld_ident = NULL
        };
      }

      if (!var_free_right) {
        SK_Tree* k = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
        assert(k != NULL);

        SK_Tree* app3 = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
        assert(app3 != NULL);

        *k = (SK_Tree){ .type = K_NODE, .left = NULL, .right = NULL, .ld_ident = NULL };

        *app3 = (SK_Tree){
          .type  = APP_NODE,
          .left  = k,
          .right = expr->right,
          .ld_ident = NULL
        };

        *app1 = (SK_Tree){
          .type  = APP_NODE,
          .left  = app2,
          .right = app3,
          .ld_ident = NULL
        };
      } else {
        *app1 = (SK_Tree){
          .type  = APP_NODE,
          .left  = app2,
          .right = _ast_expr_convert_sk(arena, expr->right, var),
          .ld_ident = NULL
        };
      }

      return app1;
    }
    case LD_NODE: {
      if (strcmp(expr->ld_ident->token->str, var->token->str) == 0) {
        SK_Tree* app2, *s, *k1, *k2;
        app2 = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
        s    = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
        k1   = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
        k2   = (SK_Tree*)arena_alloc(arena, sizeof(struct sk_tree));
        assert(app2 != NULL && s != NULL && k1 != NULL && k2 != NULL);

        *s  = (SK_Tree){ .type = S_NODE, .left = NULL, .right = NULL, .ld_ident = NULL };
        *k2 = (SK_Tree){ .type = K_NODE, .left = NULL, .right = NULL, .ld_ident = NULL };
        *k1 = (SK_Tree){ .type = K_NODE, .left = NULL, .right = NULL, .ld_ident = NULL };

        *app2 = (SK_Tree){ .type = APP_NODE, .left = s, .right = k2, .ld_ident = NULL };
        *expr = (SK_Tree){ .type = APP_NODE, .left = app2, .right = k1, .ld_ident = NULL };
      }
      return expr;
    }
    default: {
      return expr;
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

  fprintf(stderr, "[SK CONVERTER]: free var set function reached its end. Something went wrong!\n");
  exit(1);
  return false;
}

bool _ast_in_free_var_set_sk(SK_Tree* expr, ASTN_Ident* var) {
  if (expr == NULL || var == NULL)
    return false;

  switch (expr->type) {
    case APP_NODE: { 
      return (
           _ast_in_free_var_set_sk(expr->left, var)
        || _ast_in_free_var_set_sk(expr->right, var)
      );
    }
    case LD_NODE: {
      return strcmp(expr->ld_ident->token->str, var->token->str) == 0;
    }
    default: {
      return false;
    }
  }
}

void _sk_print_expr(SK_Tree* expr, size_t depth, IdentList* list) {
  assert(expr != NULL);
  
  IdentList* node = list;
  for (size_t i = 0; i < depth; i++, node = node->next)
    printf("%s   ", node->value ? " " : "│");
  
  printf("%s ", node->value ? "└──" : "├──");
  
  switch (expr->type) {
    case APP_NODE: {
      printf("@\n");
      
      list = _ident_list_append(list, false);
      _sk_print_expr(expr->left, depth + 1, list);
      (void)_ident_list_remove(&list);
      
      list = _ident_list_append(list, true);
      _sk_print_expr(expr->right, depth + 1, list);
      (void)_ident_list_remove(&list);
      
      break;
    }
    case REF_NODE: {
      printf("&%s\n", expr->left->ld_ident->token->str);
      break;
    }
    case LD_NODE: {
      printf("%s\n", expr->ld_ident->token->str);
      break;
    }
    case K_NODE: {
      printf("K\n");
      break;
    }
    case S_NODE: {
      printf("S\n");
      break;
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
