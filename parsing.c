#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpc.h"

#ifdef _WIN32
#include <string.h>

static char buffer[2048];

char* readline(char* prompt) {
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer) + 1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy) - 1] = '\0';
  return cpy;
}

void add_history(char* unused) {}

#else
#include <editline/readline.h>
#endif

#ifdef linux
#include <editline/history.h>
#endif

/* Lval types */
enum { LVAL_NUM, LVAL_ERR };

/* Error types */
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };

/* Lisp value */
typedef struct {
  int type;
  long num;
  int err;
} lval;

lval lval_num(long x) {
  lval v;
  v.type = LVAL_NUM;
  v.num = x;
  return v;
}

lval lval_err(int x) {
  lval v;
  v.type = LVAL_ERR;
  v.err = x;
  return v;
}

void lval_print(lval v) {
  switch (v.type) {
    case LVAL_NUM: printf("%li", v.num); break;
    case LVAL_ERR:
      if (v.err == LERR_DIV_ZERO) {
        printf("Error: Division By Zero!");
      }
      if (v.err == LERR_BAD_OP) {
        printf("Error: Invalid Operator!");
      }
      if (v.err == LERR_BAD_NUM) {
        printf("Error: Invalid Number!");
      }
      break;
  }
}

void lval_println(lval v) { lval_print(v); putchar('\n'); }

long eval_op(lval x, char* op, lval y) {
  if (x.type == LVAL_ERR) { return x; }
  if (y.type == LVAL_ERR) { return y; }
  
	
  if (strcmp(op, "+") == 0) { lval_num(return x.num + y.num); }
  if (strcmp(op, "-") == 0) { lval_num(return x.num - y.num); }
  if (strcmp(op, "*") == 0) { lval_num(return x.num * y.num); }
  if (strcmp(op, "/") == 0) { 
    return y == 0 
      ? lval_err(LERR_DIV_ZERO)
      : lval_num(x.num / y.num); 
  }
  if (strcmp(op, "%") == 0) { return lval_num(x.num % y.num); }
  if (strcmp(op, "^") == 0) { return lval_num(pow(x.num, y.num)); }
  if (strcmp(op, "min") == 0) { return lval_num(x.num < y.num ? x.num : y.num); }
  if (strcmp(op, "max") == 0) { return lval_num(x.num < y.num ? y.num : x.num); }
  
  return lval_err(LERR_BAD_OP);
}

long eval(mpc_ast_t* t) {
  if (strstr(t->tag, "number")) {
    return atoi(t->contents);
  }

  char* op = t->children[1]->contents;
  
  long x = eval(t->children[2]);
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }
  
  return x;
}

int main(int argc, char** argv) {
  /* Parser */
  mpc_parser_t* Number   = mpc_new("number");
  mpc_parser_t* Operator = mpc_new("operator");
  mpc_parser_t* Expr     = mpc_new("expr");
  mpc_parser_t* Lispy    = mpc_new("lispy");
  
  //operator : '+' | '-' | '*' | '/' | '%' | '^' ;  
  mpca_lang(MPCA_LANG_DEFAULT,
    "						                           \
      number   : /-?([0-9]+\\.)?[0-9]+/ ;	                           \
      operator : '+' | '-' | '*' | '/' | '%' | '^' | \"min\" | \"max\" ;   \
      expr     : <number> | '(' <operator> <expr>+ ')' ;                   \
      lispy    : /^/ <operator> <expr>+ /$/ ;                              \
    ",
    Number, Operator, Expr, Lispy);
  
  puts("Lispy Version 0.0.0.0.1");
  puts("Press Ctrl+d to Exit\n");

  char* input;
  while((input = readline("λ> ")) != NULL) {
    add_history(input);
    
    /* Parse user input */
    mpc_result_t r;
    if (mpc_parse("<stdin>", input, Lispy, &r)) {
      //mpc_ast_print(r.output);
      long result = eval(r.output);
      printf("%li\n", result);
      mpc_ast_delete(r.output);
    } else {
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    
    free(input);
  }

  mpc_cleanup(4, Number, Operator, Expr, Lispy);
  return 0;
}
