#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>

#include "ll.h"

#define BAUDRATE B38400
#define SERIAL_PORT_SEND "/dev/ttyS10"
#define SERIAL_PORT_RECV "/dev/ttyS11"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

static struct termios old_termio;
static int serial_port_fd;

void set_config(int role) {
  if (role == 0) {
    serial_port_fd = open(SERIAL_PORT_SEND, O_RDWR | O_NOCTTY);
    if (serial_port_fd < 0) {
      perror(SERIAL_PORT_SEND);
      exit(-1);
    }
  } else {
    serial_port_fd = open(SERIAL_PORT_RECV, O_RDWR | O_NOCTTY);
    if (serial_port_fd < 0) {
      perror(SERIAL_PORT_RECV);
      exit(-1);
    }
  }

  struct termios new_termio;

  if (tcgetattr(serial_port_fd, &old_termio) == -1) {
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&new_termio, sizeof(new_termio));
  new_termio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  new_termio.c_iflag = IGNPAR;
  new_termio.c_oflag = 0;
  new_termio.c_lflag = 0;
  new_termio.c_cc[VTIME] = 30;
  new_termio.c_cc[VMIN] = 0;

  tcflush(serial_port_fd, TCIOFLUSH);

  if (tcsetattr(serial_port_fd, TCSANOW, &new_termio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }
}

void reset_config() {
  sleep(1);

  if (tcsetattr(serial_port_fd, TCSANOW, &old_termio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(serial_port_fd);
}

int main(int argc, char *argv[]) {

  if (!(argc == 2 && strcmp(argv[1], "receiver") == 0) &&
      !(argc == 3 && strcmp(argv[1], "emitter") == 0)) {
    fprintf(stderr, "Usage:\t./recv SerialPort File\n\tex: nserial /dev/tty11 "
                    "pinguim.jpg\n");
    exit(-1);
  }

  bool role = -1;
  if (argc == 2 && strcmp(argv[1], "emitter") == 0) {
    role = TRANSMITTER;
  } else if (strcmp(argv[1], "receiver") == 0) {
    role = RECEIVER;
  }

  atexit(reset_config);

  set_config(role);

  llopen(serial_port_fd, role);

  return 0;
}
