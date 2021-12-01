#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "defines.h"
#include "ll.h"
#include "read.h"
#include "send.h"
#include "state.h"

static struct termios old_termio;
static bool role = -1;

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

int llopen(int port, bool role_p) {

  int ret = -1;
  int fd = -1;

  role = role_p;
  ret = set_config(port);

  if (ret == -1) {
    fprintf(stderr, "Error opening serial port\n");
    exit(-1);
  } else {
    fd = ret;
  }

  if (role == TRANSMITTER) {

    for (int i = 0; i < NUM_TRIES; i++) {

      ret = writeSupervisionAndRetry(fd, A_SEND_ADDR, C_SET);

      if (ret != 0) {
        continue;
      }

      printf("Sent SET frame\n");

      ret = readSupervisionFrame(fd);

      if (ret != SU_FRAME_SIZE || get_ctrl() != C_UA) {
        continue;
      } else {
        printf("Read UA frame\n");
        return fd;
      }
    }

    return -1;

  } else if (role == RECEIVER) {
    int ret;
    for (int i = 0; i < NUM_TRIES; i++) {

      ret = readSupervisionFrame(fd);

      if (ret != SU_FRAME_SIZE || get_ctrl() != C_SET) {
        continue;
      }

      printf("Read SET frame\n");

      ret = writeSupervisionAndRetry(fd, A_RECV_ADDR, C_UA);

      if (ret != 0) {
        continue;
      } else {
        printf("Sent UA frame\n");
        return fd;
      }
    }
    return -1;
  } else {

    return UNKNOW_ROLE;
  }
}

int llread(int fd, unsigned char *buffer) {

  printf("Reading\n");

  return 0;
}

int llwrite(int fd, unsigned char *buffer, unsigned int length) {
  printf("Writing\n");
  return 0;
}

static int reset_config(int fd) {

  if (tcsetattr(fd, TCSANOW, &old_termio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  return close(fd);
}

int llclose(int fd) {
  int ret;

  if (role == TRANSMITTER) {

    for (int i = 0; i < NUM_TRIES; i++) {
      ret = writeSupervisionAndRetry(fd, A_SEND_ADDR, C_DISC);

      if (ret != 0) {
        continue;
      }

      printf("Wrote DISC frame\n");

      ret = readSupervisionFrame(fd);

      if (ret != SU_FRAME_SIZE || get_ctrl() != C_DISC) {
        continue;
      }

      printf("Read DISC frame\n");

      ret = writeSupervisionAndRetry(fd, A_SEND_ADDR, C_UA);
      if (ret != 0) {
        continue;
      } else {
        printf("Wrote UA frame\n");
        sleep(2);
        return reset_config(fd);
      }
    }

  } else if (role == RECEIVER) {

    for (int i = 0; i < NUM_TRIES; i++) {

      ret = readSupervisionFrame(fd);

      if (ret != SU_FRAME_SIZE || get_ctrl() != C_DISC) {
        continue;
      }

      printf("Read DISC frame\n");

      ret = writeSupervisionAndRetry(fd, A_RECV_ADDR, C_DISC);

      if (ret != 0) {
        continue;
      }

      printf("Wrote DISC frame\n");

      ret = readSupervisionFrame(fd);

      if (ret != SU_FRAME_SIZE || get_ctrl() != C_UA) {
        continue;
      } else {
        printf("Read UA frame\n");
        return reset_config(fd);
      }
    }

  } else {
    return -1;
  }

  return -1;
}
