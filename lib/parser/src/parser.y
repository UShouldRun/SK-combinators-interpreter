%{
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "arena.h"
#include "ast.h"
#include "parser.tab.h"

extern const char* filename;

extern uint32_t yylineno, yyleng, current_column;
extern AST*  ast;
extern Arena arena;

extern int32_t yylex(YYSTYPE* yylval, YYLTYPE* yylloc);
void yyerror(YYLTYPE* yylloc, const char* error_msg);
%}

%define api.pure full
%define parse.error verbose
%locations

%code requires {
  #include "ast.h"
}

%union {
  AST*        ast;
  ASTN_Stmt*  stmt;
  ASTN_Expr*  expr;
  ASTN_Ident* id;
  ASTN_Token* token;
  const char* str;
}

%type <ast>   lambda
%type <stmt>  lambda_program
%type <stmt>  lambda_stmt
%type <expr>  lambda_expr
%type <expr>  lambda_expr_app
%type <expr>  lambda_expr_singleton
%type <id>    lambda_identifier_list
%type <id>    lambda_identifier
%type <token> lambda_token

%token TT_LET TT_IDENTIFIER TT_ASSIGN TT_LAMBDA TT_COMMA TT_ARROW TT_LPAREN TT_RPAREN TT_SEMI

%start lambda

%%

lambda:
    lambda_program
    { ast = ast_create(arena, filename, $1); }
  ;

lambda_program:
    lambda_stmt lambda_program
    { $$ = astn_add_stmt($1, $2); }
  | lambda_stmt
    { $$ = astn_add_stmt($1, NULL); }
  ;

lambda_stmt:
    TT_LET lambda_identifier TT_ASSIGN lambda_expr TT_SEMI
    { $$ = astn_create_stmt(arena, $2, $4, @1.first_line, @1.first_column, @5.last_line, @5.last_column); }
  ;

lambda_expr:
    TT_LAMBDA lambda_identifier_list TT_ARROW lambda_expr
    { $$ = astn_create_expr_abs(arena, $2, $4, @1.first_line, @1.first_column, @4.last_line, @4.last_column); }
  | lambda_expr_app
    { $$ = $1; }
  ;

lambda_expr_app:
    lambda_expr_singleton
    { $$ = $1; }
  | lambda_expr_app lambda_expr_singleton 
    { $$ = astn_create_expr_app(arena, $1, $2, @1.first_line, @1.first_column, @2.last_line, @2.last_column); }

lambda_expr_singleton:
    TT_LPAREN lambda_expr TT_RPAREN
    { $$ = $2; }
  | lambda_identifier
    { $$ = astn_create_expr_ident(arena, $1); }
  ;

lambda_identifier_list:
    lambda_token TT_COMMA lambda_identifier_list
    { $$ = astn_create_ident(arena, $1, $3); }
  | lambda_token
    { $$ = astn_create_ident(arena, $1, NULL); }
  ;

lambda_identifier:
    lambda_token
    { $$ = astn_create_ident(arena, $1, NULL); }
  ;

lambda_token:
    TT_IDENTIFIER
    { $$ = astn_create_token(arena, yylval.str, @1.first_line, @1.first_column, @1.last_column); }
  ;

%%

void yyerror(YYLTYPE* yylloc, const char* error_msg) {
  assert(yylloc != NULL);

  fprintf(stderr, "[PARSER]: %s in file %s at %u", error_msg, filename, yylloc->first_line);
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    fprintf(stderr, "Error: Could not access or find the source file: %s\n", filename);
    return;
  }
  char line[1024];
  for (
    int32_t current_line = 1;
    fgets(line, sizeof(line), file) && current_line != yylloc->first_line;
    current_line++
  );
  fclose(file);

  fprintf(stderr, "%s", line);
  for (int32_t i = 1; i < yylloc->first_column; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "%s^", "\033[31m");
  int32_t s_word = yylloc->last_column - yylloc->first_column + 1;
  for (int32_t i = 1; i < s_word; i++)
    fprintf(stderr, "~");
  fprintf(stderr, "%s\n", "\033[0m");
}
