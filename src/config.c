#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "config.h"
#include "datalink-defines.h"

static struct termios old_termio;

int set_config(int port) {

  char serial_port[15];
  sprintf(serial_port, "/dev/ttyS%d", port);

  int serial_port_fd = open(serial_port, O_RDWR | O_NOCTTY);

  if (serial_port_fd < 0) {
    perror(serial_port);
    exit(-1);
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

  return serial_port_fd;
}

int reset_config(int fd) {

  if (tcsetattr(fd, TCSANOW, &old_termio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  return close(fd);
}
