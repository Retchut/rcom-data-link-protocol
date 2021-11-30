#include <string.h>
#include <unistd.h>

#include "defines.h"
#include "write.h"

int writeInformationFrame(int fd, unsigned char addr, unsigned char cmd,
                          unsigned char *infoPtr, size_t infoSize) {

  unsigned char frame[I_FRAME_SIZE(infoSize)];

  frame[0] = FLAG;
  frame[1] = addr;
  frame[2] = cmd;
  frame[3] = BCC1(addr, cmd);

  // TODO: Byte stuff the info part of the message
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

// TODO: Maybe make it static
unsigned char buildBCC2(unsigned char *data, size_t size) {
  unsigned char bcc2 = data[0];

  for (size_t p = 0; p < size; p++) {
    bcc2 ^= data[p];
  }

  return bcc2;
}

int writeSupervisionAndRetry(int fd, unsigned char msg_addr,
                             unsigned char msg_ctrl) {
  int ret = 0;

  for (int i = 0; i < NUM_TRIES; i++) {
    ret = writeSupervisionFrame(fd, msg_addr, msg_ctrl);
    if (ret != SU_FRAME_SIZE) {
      sleep(2);
    } else {
      return 0;
    }
  }

  return -1;
}
