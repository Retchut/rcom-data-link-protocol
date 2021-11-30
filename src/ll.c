#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <time.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "./ll.h"

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

  unsigned char buf[SU_FRAME_SIZE] = "ABCDE";

  /*  buf[0] = FLAG;
    buf[1] = msg_addr;
    buf[2] = msg_ctrl;
    buf[3] = BCC1(msg_addr, msg_ctrl);
    buf[4] = FLAG;
    */

  return write(fd, &buf, SU_FRAME_SIZE);
}

int readSupervisionFrame(int fd) {
  unsigned char buf;
  int count = 0, ret = -1;
  while (count < 5) {

    ret = read(fd, &buf, 1);

    if (ret <= 0) {
      exit(-1);
    }

    printf("Received a %c\n", buf);
    count++;
    sleep(1);
  }

  return SU_FRAME_SIZE;
}

int llopen(int fd, bool role) {
  if (role == TRANSMITTER) {
    int ret;

    ret = writeSupervisionFrame(fd, A_SEND_CMD_ADDR, C_SET);

    printf("Sent SET frame\n");

    if (ret != SU_FRAME_SIZE) {
      perror("writeSupervisionFrame");
      exit(-1);
    }

    sleep(1);

    ret = readSupervisionFrame(fd);

    if (ret != SU_FRAME_SIZE) {
      perror("readSupervisionFrame");
      exit(-1);
    }

    printf("Received UA frame");

    return fd;

  } else if (role == RECEIVER) {
    int ret;

    ret = readSupervisionFrame(fd);

    if (ret != SU_FRAME_SIZE) {
      perror("readSupervisionFrame");
      exit(-1);
    }

    printf("Received SET frame");

    ret = writeSupervisionFrame(fd, A_RECV_CMD_ADDR, C_UA);

    if (ret = SU_FRAME_SIZE) {
      perror("writeSupervisionFrame");
      exit(-1);
    }

    printf("Sent UA frame");

    return 0;
  } else {
    return -1;
  }
}
