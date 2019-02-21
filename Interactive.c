#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <string.h>

static char buffer[2048];

char* readline(char* prompt){
  fputs(prompt, stdout);
  fgets(buffer, 2048, stdin);
  char* cpy  malloc(strlen(buffer)+1);
  strcpy(cpy, buffer);
  cpy[strln(cpy)-1] = '\0';
  return cpy;
}

void add_histroy(char* unused){}

#else
#include <editline/readline.h>
#endif

int main(int argc, char** argv){

  puts("Lisp version 0.0.0.3");
  puts("Press Ctrl+C to exit\n");

  while (1){
    char* input = readline("Lisp> ");

    add_history(input);

    printf("I don't understand the meaning of:%s\n", input);

    free(input);
  }

return 0;

}
