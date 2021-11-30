#include <stdio.h>
#include <unistd.h>

#include "defines.h"
#include "ll.h"
#include "read.h"
#include "state.h"
#include "write.h"

int llopen(int fd, bool role) {
  if (role == TRANSMITTER) {
    int ret;

    for (int i = 0; i < 3; i++) {
      for (int i = 0; i < 3; i++) {
        ret = writeSupervisionFrame(fd, A_SEND_CMD_ADDR, C_SET);
        if (ret != SU_FRAME_SIZE) {
          sleep(2);
        } else {
          printf("Sent SET frame\n");
          break;
        }
      }

      if (ret != SU_FRAME_SIZE) {
        fprintf(stderr, "Error writing SET Frame\n");
        continue;
      }

      ret = readSupervisionFrame(fd);

      if (ret != SU_FRAME_SIZE || get_ctrl() != C_UA) {
        fprintf(stderr, "Error reading UA Frame\n");
      } else {
        printf("Received UA frame\n");
        return fd;
      }
    }

    fprintf(stderr, "Error establishing communication\n");

    return -1;

  } else if (role == RECEIVER) {
    int ret;
    for (int i = 0; i < 3; i++) {
      for (int i = 0; i < 3; i++) {
        ret = readSupervisionFrame(fd);
        if (ret != SU_FRAME_SIZE) {
          sleep(2);
        } else {
          printf("Received SET frame\n");
          break;
        }
      }

      if (ret != SU_FRAME_SIZE || get_ctrl() != C_SET) {
        fprintf(stderr, "Error reading SET frame\n");
        continue;
      }

      for (int i = 0; i < 3; i++) {
        ret = writeSupervisionFrame(fd, A_RECV_CMD_ADDR, C_UA);
        if (ret != SU_FRAME_SIZE) {
          sleep(2);
        } else {
          printf("Sent UA frame\n");
          break;
        }
      }

      if (ret != SU_FRAME_SIZE) {
        fprintf(stderr, "Error writing UA frame\n");
      } else {
        printf("Wrote UA Frame \n");
        return fd;
      }
    }
    return -1;
  } else {
    return -1;
  }
}
