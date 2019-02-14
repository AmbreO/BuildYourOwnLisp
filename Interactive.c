#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>

int main(int argc, char** argv){

  puts("Lisp version 0.0.0.2");
  puts("Press Ctrl+C to exit\n");

  while (1){
    char* input = readline("Lisp> ");

    add_history(input);

    printf("I don't understand the meaning of:%s\n", input);

    free(input);
  }

return 0;

}
