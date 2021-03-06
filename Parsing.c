#include "mpc.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <string.h>

static char buffer[2048];

char* readline(char* prompt){
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy = malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strlen(cpy)-1] = '\0';
  return cpy;
}

void add_history(char* unused){}

#else
#include <editline/readline.h>
#endif

/*Function that raises x to the power of y*/
long raise(long base, long power){
  /* If the power is less than one,
  we'll return zero since we can't represent decimals with a long
  */
  if (power < 0) { return 0; }
  /* Return 1 for x^0 or 1^y since these are default cases */
  else if ((power == 0) | (base == 1)) { return 1; }
  else{
    long result = 1;
    for (; power > 0; --power)
      result *= base;
    return result;
  }
}

/*Create enumeration of possible error types*/
enum {LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM};

/*Create enumeration of possible lval types*/
enum {LVAL_NUM, LVAL_ERR, LVAL_SYM, LVAL_SEXPR};

/*Declare new lval struct*/
typedef struct lval{
  int type;
  long num;
  /* Error and Symbol types have some string data */
  char* err;
  char* sym;
  /* Count and pointer list to a list of "lval*" */
  int count;
  struct lval** cell;
} lval;

/* Construct a pointer to a new Number pointer */
lval* lval_num(long x){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_NUM;
  v->num = x;
  return v;
}

/* Construct a pointer to a new Error pointer */
lval* lval_err(char* m){
  lval* v = malloc(sizeof(lva));
  v->type = LVAL_ERR;
  v->err = malloc(strlen(m) + 1);
  strcpy(v->err, m);
  return v;
}

/* Construct a pointer to a new Symbol lval */
lval* lval_sym(char* s){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SYM;
  v->sym = malloc(strlen(s) + 1);
  strcpy(v->sym, s);
  return v;
}

/* A pointer to a new empty Sexpr lval */
lval* lval_sexpr(void){
  lval* v = malloc(sizeof(lval));
  v->type = LVAL_SEXPR;
  v->count = 0;
  v->cell = NULL;
  return v;
}

void lval_del(lval* v){
  switch (v->type){
    /* Do nothing special for number type */
    case LVAL_NUM: break;

    /* For Err or Sym free the string data */
    case LVAL_ERR: free(v->err); break;
    case LVAL_SYM: free(v->sym); break;

    /* If Sexpr, then delete all elements inside */
    case LVAL_SEXPR:
      for (int i = 0; i < v->count; i++){
        lval_del(v->cell[i]);
      }
      /* Also free the memory allocated to contain the pointers */
      free(v->cell);
    break;
  }
  /* Free the memory allocated for the "lval" struct itself */
  free(v);
}

/*Print a "lval"*/
void lval_print(lval v){
  switch (v.type){
    /*In the case the type is a number, print it*/
    /*Then 'break' out of the switch*/
    case LVAL_NUM: printf("%li", v.num); break;

    /*In the case the type is an error*/
    case LVAL_ERR:
    /*Check what type of error it is and print it*/
    if (v.err == LERR_DIV_ZERO){
      printf("Error:Division by zero!");
    }
    if (v.err == LERR_BAD_OP){
      printf("Error:Invalid operator");
    }
    if (v.err == LERR_BAD_NUM){
      printf("Error:Invalid number");
    }
  }
}

/*Print a "lval" followed by a newline*/
void lval_println(lval v){lval_print(v); putchar('\n');}

/* Use operator string to see which operation to perform. */
lval eval_op(lval x, char* op, lval y){
  /*If either value is an error, return it*/
  if (x.type == LVAL_ERR){return x;}
  if (y.type == LVAL_ERR){return y;}

  /*Otherwise, do maths on the number values*/
  if (strcmp(op, "+") == 0) { return lval_num(x.num + y.num); } //Addition
  if (strcmp(op, "-") == 0) { return lval_num(x.num - y.num); } //Subtraction
  if (strcmp(op, "*") == 0) { return lval_num(x.num * y.num); } //Multiplication
  if (strcmp(op, "/") == 0) {                                   //Division
    /*If second operand is equal to zero, return an error*/
    return y.num == 0
      ? lval_err(LERR_DIV_ZERO)
      : lval_num(x.num / y.num);
  }
  if (strcmp(op, "%") == 0) {                                   //Remainder of Division
    return y.num == 0
    ? lval_err(LERR_DIV_ZERO)
    : lval_num(x.num - (x.num / y.num) * y.num);
  }
  //if (strcmp(op, "^") == 0) { return raise(x, y); }     //Raises x to the power of y
  return lval_err(LERR_BAD_OP);
}

lval* lval_read_num(mpc_ast_t* t){
  errno = 0;
  long x = strtol(t->contetns, NULL, 10);
  return errno != ERANGE ?
    lval_num(x) : lval_err("Invalid number");
}

lval* lval_read(mpc_ast_t* t){
  /* If symbol or Number return converison to that type */
  if (strstr(t->tag, "number")){ return lval_read_num(t); }
  if (strstr(t->tag, "symbol")){ return lval_sym(t->contents); }

  /* If root (>) or sexpr then create empty list */
  lval* x = NULL;
  if (strcmp(t->tag, ">") == 0){ x= lval_sexpr(): }
  if (strstr(t->tag, "sexpr")) { x= lval_sexpr(); }

  /* Fill this list with any valid expression contained within */
  for (int i = 0; i < t->children_num; i++){
    if (strcmp(t->children[i]->contents, "(") == 0){ continue; }
    if (strcmp(t->children[i]->contents, ")") == 0){ continue; }
    if (strcmp(t->children[i]->tag, "regex") == 0){ continue; }
    x = lval_add(x, lval_read(t->children[i]));
  }
  return x;
}

lval* lval_add(lval* v, lval* x){
  v->count++;
  v->cell = realloc(v->cell, sizeof(lval*) * v->count);
  v->cell[v->count-1] = x;
  return v;
}

lval eval(mpc_ast_t* t){
  /* If tagged as number return it directly. */
  if (strstr(t->tag, "number")){
    /*Check if there is some error in conversion*/
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? lval_num(x) : lval_err(LERR_BAD_NUM);
  }

  /* The operator is always second child. */
  char* op = t->children[1]->contents;

  /* We store the third child in 'x' */
  lval x = eval(t->children[2]);

  /* Iterate the remaining children and combining. */
  int i = 3;
  while (strstr(t->children[i]->tag, "expr")) {
    x = eval_op(x, op, eval(t->children[i]));
    i++;
  }
  return x;
}

int main(int argc, char** argv){

  //Create some parsers
  mpc_parser_t* Number = mpc_new("number");
  mpc_parser_t* Symbol = mpc_new("symbol");
  mpc_parser_t* Sexpr = mpc_new("sexpr");
  mpc_parser_t* Expr = mpc_new("expr");
  mpc_parser_t* Lispy = mpc_new("lispy");

  //Defining them with the following language
  mpca_lang(MPCA_LANG_DEFAULT,
  "                                               \
    number  : /-?[0-9]+/ ;                        \
    symbol  : '+' | '-' | '*' | '/' | '%' | '^' ; \
    sexpr   : '(' <expr>* ')' ;                   \
    expr    : <number> | <symbol> | <sexpr> ;     \
    lispy   : /^/ <expr>* /$/ ;                   \
  ",
  Number, Symbol, Sexpr, Expr, Lispy);

  puts("Lisp version 0.0.1.1");
  puts("Press Ctrl+C to exit\n");

  while (1){
    char* input = readline("Lisp> ");

    add_history(input);

    //Attempt to parse the user input
    mpc_result_t r;
    if(mpc_parse("<stdin>", input, Lispy, &r)){
      lval result = eval(r.output);
      lval_println(result);
      mpc_ast_delete(r.output);
    } else {
      //Otherwise print the error
      mpc_err_print(r.error);
      mpc_err_delete(r.error);
    }
    free(input);
  }

//Undefine and delete our parsers
mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lispy);

return 0;

}
