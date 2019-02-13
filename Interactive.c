#include <stdio.h>

static char input[2048];

  int main(int argc, char** argv){

  puts("Lisp version 0.0.0.1");
  puts("Press Ctrl+C to exit\n");

  while (1){
    fputs("Lisp> ", stdout);

    fgets(input, 2048, stdin);

    printf("I don't understand the meaning of:%s", input);
  }

return 0;

}
