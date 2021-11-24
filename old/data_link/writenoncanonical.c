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
#define SERIAL_PORT "/dev/ttyS10"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP = FALSE;

int main(int argc, char **argv) {
  int fd, c, res;
  struct termios oldtio, newtio;
  char buf[255];
  int i, sum = 0, speed = 0;

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

  newtio.c_cc[VTIME] = 30; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 0;   /* blocking read until 1 chars received */

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

  const char SET[] = {0x7E, 0x03, 0x03, 0x03 ^ 0x03, 0x7E};
  enum msg { F = 0x7E, A = 0x01, C = 0x07, BCC = A ^ C };
  enum flag_state { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP };

  enum flag_state flag = START;

  int input;
  int counter = 0;

  while (counter < 3) { // TODO: Implement timeout

    write(fd, SET, 5); // SET channel for communication

    // State machine for parsing UA signal
    while (flag != STOP) {

      input = read(fd, &buf, 1);
      printf("WRITE RECEIVED: %s", buf);

      switch (input) {
      case F:
        if (flag == BCC_OK)
          flag = STOP;
        else
          flag = FLAG_RCV;
        break;
      case A:
        if (flag == FLAG_RCV)
          flag = A_RCV;
        else
          flag = START;
        break;
      case C:
        if (flag == A_RCV)
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

      sleep(1);
      counter++;
    }
  }

  /*
    O ciclo FOR e as instruções seguintes devem ser alterados de modo a
    respeitar o indicado no guião
  */

  if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);
  return 0;
}