void yyerror(const char *msg) {
  fprintf(stderr, "%s\n", msg);
  exit(1);
}
