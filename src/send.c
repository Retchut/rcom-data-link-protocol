#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "send.h"
#include "read.h"
#include "datalink-defines.h"
#include "utils.h"

int writeInformationFrame(int fd, unsigned char addr, unsigned char *info_ptr,
                          size_t info_size, int msg_nr) {

  if (info_size <= 0)
    return -1;

  unsigned char stuffed_info[info_size * 2];
  int stuffed_size = stuff_data(info_ptr, info_size, stuffed_info);

  int ret = -1;
  unsigned char frame[I_FRAME_SIZE(stuffed_size)];

  frame[0] = FLAG;
  frame[1] = addr;
  frame[2] = C_S(msg_nr);
  frame[3] = BCC1(addr, C_S(msg_nr));

  memcpy(frame + 4, stuffed_info, stuffed_size);

  unsigned char original_bcc2 = build_BCC2(info_ptr, info_size);
  unsigned short stuffed_bcc2;

  if (original_bcc2 == FLAG) {
    stuffed_bcc2 = (ESCAPE << 8) | FLAG_ESCAPE;
  } else if (original_bcc2 == ESCAPE) {
    stuffed_bcc2 = (ESCAPE << 8) | ESCAPE_ESCAPE;
  } else {
    frame[stuffed_size + 4] = original_bcc2;
    frame[stuffed_size + 5] = FLAG;

    ret = write(fd, frame, I_FRAME_SIZE(stuffed_size) - 1);

    return ret == (I_FRAME_SIZE(stuffed_size) - 1) ? 0 : -1;
  }

  frame[stuffed_size + 4] = ((stuffed_bcc2 & 0xFF00) >> 8);
  frame[stuffed_size + 5] = (stuffed_bcc2 & 0x00FF);
  frame[stuffed_size + 6] = FLAG;

  ret = write(fd, frame, I_FRAME_SIZE(stuffed_size));

  return ret == I_FRAME_SIZE(stuffed_size) ? 0 : -1;
}

int writeInformationAndRetry(int fd, unsigned char addr,
                             unsigned char *info_ptr, size_t info_size,
                             int msg_nr) {

  int current_attempt = 0;
  int ret = -1;
  do {
    current_attempt++;

    ret = writeInformationFrame(fd, addr, info_ptr, info_size, msg_nr);

    if (ret != 0) {
      continue;
    }

    ret = readInformationFrameResponse(fd);

    if (ret == -1) {
      continue;
    } else if (ret == REJECTED) {
      current_attempt = 0;
      continue;
    } else {
      if (ret == ((msg_nr + 1) % 2)) {
        return info_size;
      } else {
        continue;
      }
    }

  } while (current_attempt < NUM_TRIES);

  return -1;
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

int writeSupervisionAndRetry(int fd, unsigned char msg_addr,
                             unsigned char msg_ctrl) {
  int ret = 0;

  for (int i = 0; i < NUM_TRIES; i++) {
    ret = writeSupervisionFrame(fd, msg_addr, msg_ctrl);
    if (ret != SU_FRAME_SIZE) {
      sleep(1);
    } else {
      return 0;
    }
  }

  return -1;
}
