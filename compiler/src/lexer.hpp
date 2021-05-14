#ifndef __LEXER_HPP__
#define __LEXER_HPP__

int yylex();
void yyerror(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}

#endif