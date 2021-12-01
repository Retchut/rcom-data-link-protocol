#include <stdio.h>
#include <unistd.h>

#include "defines.h"
#include "ll.h"
#include "read.h"
#include "send.h"
#include "state.h"

int llopen(int fd, bool role) {
  if (role == TRANSMITTER) {
    int ret;

    for (int i = 0; i < NUM_TRIES; i++) {

      ret = writeSupervisionAndRetry(fd, A_SEND_ADDR, C_SET);

      if (ret != 0) {
        continue;
      }

      ret = readSupervisionFrame(fd);

      if (ret != SU_FRAME_SIZE || get_ctrl() != C_UA) {
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

      if (ret != SU_FRAME_SIZE || get_ctrl() != C_SET) {
        continue;
      }

      ret = writeSupervisionAndRetry(fd, A_RECV_ADDR, C_UA);

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

int llclose_send(int fd) {
  int ret;

  for (int i = 0; i < NUM_TRIES; i++) {
    ret = writeSupervisionAndRetry(fd, A_SEND_ADDR, C_DISC);

    if (ret != 0) {
      continue;
    }

    ret = readSupervisionFrame(fd);

    if (ret != SU_FRAME_SIZE || get_ctrl() != C_DISC) {
      continue;
    } else {
      break;
    }
  }

  ret = writeSupervisionAndRetry(fd, A_SEND_ADDR, C_UA);
  if (ret != 0)
    return -1;

  return 0;
}

int llclose_recv(int fd) {
  int ret;

  for (int i = 0; i < NUM_TRIES; i++) {
    ret = readSupervisionFrame(fd);

    if (ret != SU_FRAME_SIZE || get_ctrl() != C_DISC) {
      continue;
    }

    ret = writeSupervisionAndRetry(fd, A_RECV_ADDR, C_DISC);

    if (ret != SU_FRAME_SIZE || get_ctrl() != C_DISC) {
      continue;
    } else {
      break;
    }
  }

  ret = readSupervisionFrame(fd);

  if (ret != SU_FRAME_SIZE || get_ctrl() != C_UA)
    return -1;

  return 0;
}
