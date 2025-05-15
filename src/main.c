#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "parser.tab.h"
#include "ast_priv.h"
#include "interpreter.h"

extern FILE* yyin;
extern int yylex_destroy(void);

const char* filename;
Arena arena = NULL;
AST*  ast   = NULL;

int32_t main(int32_t argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "[ERROR]: no file was passed as argument\n");
    return 1;
  }

  size_t s_filename = strlen(argv[1]);
  if (s_filename < 4) {
    fprintf(stderr, "[ERROR]: filename must have the following regular expression '*.lb'\n");
    return 1;
  }

  filename = argv[1];
  const char file_type[3] = {
    filename[s_filename - 3],
    filename[s_filename - 2],
    filename[s_filename - 1]
  };

  if (strcmp(file_type, ".ld")) {
    fprintf(stderr, "[ERROR]: filename must have the following regular expression '*.lb'\n");
    return 1;
  }

  yyin = fopen(filename, "r");
  if (yyin == NULL) {
    fprintf(stderr, "[ERROR]: could not open input file %s - %s\n", filename, strerror(errno));
    return 1;
  }

  const size_t s_arena = 1 << 20, max_nodes = 5;
  arena = arena_create_aligned(s_arena, MAX_SIZE, max_nodes);
  assert(arena != NULL);

  yyparse();
  if (ast == NULL) {
    fprintf(stderr, "[ERROR]: could not parse input file %s\n", filename);
    arena_destroy(arena);
    yylex_destroy();
    return 1;
  }

  const size_t s_table = 1 << 5;
  HashTable table = ast_check(ast, s_table);
  if (table == NULL) {
    fprintf(stderr, "[ERROR]: failed AST check of file %s\n", filename);
    arena_destroy(arena);
    yylex_destroy();
    return 1;
  } else {
    fprintf(stdout, "Successfully checked AST\n");
  }

  ast_transform(arena, ast);
  ast_print(ast);

  size_t s_roots = ast->s_stmts;
  SK_Tree** roots = ast_convert(arena, ast, table);

  skt_print(roots, s_roots);

  hashtable_free(table);
  arena_destroy(arena);
  yylex_destroy();

  return 0;
}
