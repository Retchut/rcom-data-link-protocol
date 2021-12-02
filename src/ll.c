#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "config.h"
#include "defines.h"
#include "ll.h"
#include "read.h"
#include "send.h"
#include "state.h"
#include "utils.h"

static bool role = -1;

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

      ret = writeSupervisionAndRetry(fd, A_SEND_CMD, C_SET);

      if (ret != 0) {
        continue;
      }

      ret = readSupervisionFrame(fd);

      if (ret != SU_FRAME_SIZE || get_ctrl() != C_UA ||
          get_addr() != A_RECV_RSP) {
        continue;
      } else {
        return fd;
      }
    }

    return -1;

  } else if (role == RECEIVER) {
    int ret;
    for (int i = 0; i < NUM_TRIES; i++) {

      ret = readSupervisionFrame(fd);

      if (ret != SU_FRAME_SIZE || get_ctrl() != C_SET ||
          get_addr() != A_SEND_CMD) {
        continue;
      }

      ret = writeSupervisionAndRetry(fd, A_RECV_RSP, C_UA);

      if (ret != 0) {
        continue;
      } else {
        return fd;
      }
    }
    return -1;
  } else {

    return UNKNOW_ROLE;
  }
}

int llread(int fd, unsigned char *buffer) {
  static int packet = 0;

  int ret = -1;
  unsigned char stuffed_msg[MAX_FRAME_SIZE];
  unsigned char unstuffed_msg[MAX_DATA_CHUNK_SIZE + 7];
  printf("llread 0\n");
  for (int i = 0; i < NUM_TRIES; i++) {

    ret = readInformationMessage(fd, stuffed_msg);

    if (ret == -1) {
      continue;
    }

    ret = unstuff_frame(stuffed_msg, ret, unstuffed_msg);

    size_t size = ret - 1;

    if (ret == -1) {
      continue;
    }

    unsigned char unstuffed_bcc2 = unstuffed_msg[size];
    unsigned char recv_data_bcc2 = build_BCC2(unstuffed_msg, size);

    if (unstuffed_bcc2 == recv_data_bcc2 && get_ctrl() == C_S(packet)) {
      packet++;

      ret = writeSupervisionAndRetry(fd, A_RECV_RSP, C_RR(packet % 2));

      if (ret != 0) {
        continue;
      } else {
        memcpy(buffer, unstuffed_msg, size);
        return size;
      }
    } else if (unstuffed_bcc2 == recv_data_bcc2) {
      ret = writeSupervisionAndRetry(fd, A_RECV_RSP, C_RR(packet % 2));
      tcflush(fd, TCIFLUSH);
      return -1;
    } else {
      ret = writeSupervisionAndRetry(fd, A_RECV_RSP, C_REJ(packet));
      tcflush(fd, TCIFLUSH);
      return -1;
    }
  }

  return -1;
}

int llwrite(int fd, unsigned char *buffer, unsigned int length) {

  static int frame_nr = 0;

  int ret = -1;

  for (int i = 0; i < NUM_TRIES; i++) {
    printf("Sending packet %d\n", frame_nr);
    ret = writeInformationAndRetry(fd, A_SEND_CMD, buffer, length, frame_nr);

    if (ret != -1) {
      frame_nr++;
      return ret;
    }
  }

  return -1;
}

int llclose(int fd) {
  int ret;

  if (role == TRANSMITTER) {

    for (int i = 0; i < NUM_TRIES; i++) {
      ret = writeSupervisionAndRetry(fd, A_SEND_CMD, C_DISC);

      if (ret != 0) {
        continue;
      }

      ret = readSupervisionFrame(fd);

      if (ret != SU_FRAME_SIZE || get_ctrl() != C_DISC ||
          get_addr() != A_RECV_CMD) {
        continue;
      }

      ret = writeSupervisionAndRetry(fd, A_SEND_RSP, C_UA);
      if (ret != 0) {
        continue;
      } else {
        sleep(2);
        return reset_config(fd);
      }
    }

  } else if (role == RECEIVER) {

    for (int i = 0; i < NUM_TRIES; i++) {

      ret = readSupervisionFrame(fd);

      if (ret != SU_FRAME_SIZE || get_ctrl() != C_DISC ||
          get_addr() != A_SEND_CMD) {
        continue;
      }

      ret = writeSupervisionAndRetry(fd, A_RECV_CMD, C_DISC);

      if (ret != 0) {
        continue;
      }

      ret = readSupervisionFrame(fd);

      if (ret != SU_FRAME_SIZE || get_ctrl() != C_UA ||
          get_addr() != A_SEND_RSP) {
        continue;
      } else {
        return reset_config(fd);
      }
    }

  } else {
    return UNKNOW_ROLE;
  }

  return -1;
}
