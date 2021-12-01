#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>

#include "defines.h"
#include "ll.h"

int main(int argc, char *argv[]) {

  if (!(argc == 2 && strcmp(argv[1], "receiver") == 0) &&
      !(argc == 3 && strcmp(argv[1], "emitter") == 0)) {
    fprintf(stderr, "Usage:\t./recv SerialPort File\n\tex: nserial /dev/tty11 "
                    "pinguim.jpg\n");
    exit(-1);
  }

  bool role = -1;
  if (strcmp(argv[1], "emitter") == 0) {
    role = TRANSMITTER;
  } else if (strcmp(argv[1], "receiver") == 0) {
    role = RECEIVER;
  }

  llopen(serial_port_fd, role);

  return 0;
}
