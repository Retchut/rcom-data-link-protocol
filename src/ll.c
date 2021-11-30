#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ll.h"
#include "state.h"

unsigned char buildBCC2(unsigned char *data, size_t size) {
  unsigned char bcc2 = data[0];

  for (size_t p = 0; p < size; p++) {
    bcc2 ^= data[p];
  }

  return bcc2;
}

int writeInformationFrame(int fd, unsigned char addr, unsigned char cmd,
                          unsigned char *infoPtr, size_t infoSize) {

  unsigned char frame[I_FRAME_SIZE(infoSize)];

  frame[0] = FLAG;
  frame[1] = addr;
  frame[2] = cmd;
  frame[3] = BCC1(addr, cmd);

  // check if we're not building an information frame
  memcpy(frame + 4, infoPtr, infoSize);
  frame[infoSize + 4] = buildBCC2(infoPtr, infoSize);
  frame[infoSize + 5] = FLAG;

  return write(fd, frame, infoSize + 6);
}

int writeSupervisionFrame(int fd, unsigned char msg_addr,
                          unsigned char msg_ctrl) {
  unsigned char buf[SU_FRAME_SIZE];

  buf[0] = FLAG;
  buf[1] = msg_addr;
  buf[2] = msg_ctrl;
  buf[3] = BCC1(msg_addr, msg_ctrl);
  buf[4] = FLAG;

  return write(fd, &buf, SU_FRAME_SIZE);
}

int readSupervisionFrame(int fd) {

  unsigned char buf;

  int ret = -1;

  set_state(START);
  time_t start_time = time(NULL), end_time = time(NULL);

  while (difftime(end_time, start_time) < 3 && get_state() != STOP) {

    ret = read(fd, &buf, 1);

    printf("I read a %c\n", buf);
    if (ret == -1) {
      perror("read");
      exit(-1);
    }
    if (ret == 0) {
      end_time = time(NULL);
    } else {
      end_time = time(&start_time);
    }

    handleState(buf);
  }

  return difftime(end_time, start_time) < 3 ? SU_FRAME_SIZE : -1;
}

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
