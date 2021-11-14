#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int flag = 1, conta = 0;

void atende() // atende alarme
{
  printf("alarme # %d\n", conta);
  flag = 1;
  conta++;
}

int main(int argc, char **argv) {

  (void)signal(SIGALRM, atende); // instala  rotina que atende interrupcao

  while (conta < 4) {
    if (flag) {
      alarm(2); // activa alarme de 3s
      flag = 0;
    }
  }
  printf("Vou terminar.\n");
}
