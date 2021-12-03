#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "defines.h"
#include "ll.h"
#include "read.h"
#include "state.h"

int readSupervisionFrame(int fd) {

  unsigned char buf;

  int ret = -1;

  set_state(START);
  time_t start_time = time(NULL), end_time = time(NULL);

  while (difftime(end_time, start_time) < 3 && get_state() != STOP) {

    ret = read(fd, &buf, 1);

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

int readInformationFrameResponse(int fd) {

  int ret = readSupervisionFrame(fd);

  if (ret != SU_FRAME_SIZE) {
    return -1;
  }

  unsigned char ctrl = get_ctrl();

  if (ctrl == C_RR0 || ctrl == C_RR1) {
    return (ctrl >> 7);
  }

  if (ctrl == C_REJ0 || ctrl == C_REJ1) {
    return REJECTED;
  }

  else {
    return -1;
  }
}

int readInformationMessage(int fd, unsigned char *stuffed_msg) {
  unsigned char byte;

  int idx = 0;
  int ret = -1;

  set_state(START);
  time_t start_time = time(NULL), end_time = time(NULL);
  while (difftime(end_time, start_time) < 3 && get_state() != STOP) {

    ret = read(fd, &byte, 1);

    if (ret == -1) {
      perror("read");
      exit(-1);
    }
    if (ret == 0) {
      end_time = time(NULL);
    } else {
      end_time = time(&start_time);
    }

    if (get_state() == DATA_RCV) {
      stuffed_msg[idx++] = byte;
    }

    handleState(byte);
  }
  int stuffed_size=idx-1;
  printf("read stuffed:\n");
  printf("1018: %X\t1019: %X\t1020: %X\t 1021: %X\t1022: %X\t1023: %X\n", stuffed_msg[stuffed_size-6], stuffed_msg[stuffed_size-5], stuffed_msg[stuffed_size-4], stuffed_msg[stuffed_size-3], stuffed_msg[stuffed_size-2], stuffed_msg[stuffed_size-1]);

  return difftime(end_time, start_time) < 3 ? idx - 1 : -1;
}
