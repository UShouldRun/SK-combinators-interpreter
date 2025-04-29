%{
#include <unistd.h>
#include "parser.tab.h"

extern Arena arena;
int32_t current_column = 1;
#define YY_USER_ACTION \
  yylloc->first_line   = yylineno; \
  yylloc->first_column = current_column; \
  yylloc->last_line    = yylineno; \
  yylloc->last_column  = current_column + yyleng - 1; \
  current_column      += yyleng;
%}

%option yylineno
%option caseless
%option noinput nounput
%option noyywrap
%option bison-bridge bison-locations

alpha      [_a-zA-Z]
digit      [0-9]

id         {alpha}({alpha}|{digit})*

whitespace [ \t\r]+
comment    ("//".*\n|"/*"([^*]|\*+[^*/])*\*+"/")

%%

"let"         { return TT_LET; }
"="           { return TT_ASSIGN; }
"\\"          { return TT_LAMBDA; }
","           { return TT_COMMA; }
"->"          { return TT_ARROW; }
"("           { return TT_LPAREN; }
")"           { return TT_RPAREN; }
";"           { return TT_SEMI; }

{whitespace}  ;
\n            { current_column = 1; }
{comment}     ;
{id}          { yylval->str = arena_strdup(arena, yytext); return TT_IDENTIFIER; }

.             ;

%%
