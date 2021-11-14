/*Non-Canonical Input Processing*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP = FALSE;

int parse_set(int fd) {
  // State machine for parsing SET signal
  enum flag_state { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP };
  enum SET_msg { F = 0x7E, A = 0x03, C = 0x03, BCC = A ^ C };
  enum flag_state flag = START;

  int input;
  int counter = 0;

  while (flag != STOP && counter < 5) {
    input = read(fd, input, 1);
    if (input == NULL)
      continue;
    printf("READ_RECEIVED: %s", input);
    counter++;

    switch (input) {
    case F:
      if (flag == BCC_OK)
        flag = STOP;
      else
        flag = FLAG_RCV;
      break;
    case A: // or case C
      if (flag == FLAG_RCV)
        flag = A_RCV;
      else if (flag == A_RCV) // for when the input is C
        flag = C_RCV;
      else
        flag = START;
      break;
    case BCC:
      if (flag == C_RCV)
        flag = BCC_OK;
      else
        flag = START;
      break;
    default:
      flag = START;
      break;
    }
  }

  return flag == STOP ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char **argv) {
  int fd, c, res;
  struct termios oldtio, newtio;
  char buf[255];

  if ((argc < 2) || ((strcmp("/dev/ttyS10", argv[1]) != 0) &&
                     (strcmp("/dev/ttyS11", argv[1]) != 0))) {
    printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
    exit(1);
  }

  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */

  fd = open(argv[1], O_RDWR | O_NOCTTY);
  if (fd < 0) {
    perror(argv[1]);
    exit(-1);
  }

  if (tcgetattr(fd, &oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 0; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 0;  /* blocking read until 5 chars received */

  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) próximo(s) caracter(es)
  */

  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  printf("New termios structure set\n");
  while (1) {
    parse_set(fd);
  }

  const char UA[] = {0x7E, 0x01, 0x07, 0x01 ^ 0x07, 0x7E};
  write(fd, UA, 5); // Sends confirmation

  /*
    O ciclo WHILE deve ser alterado de modo a respeitar o indicado no guião
  */

  tcsetattr(fd, TCSANOW, &oldtio);
  close(fd);
  return 0;
}
