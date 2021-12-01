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
    return 0;
  }

  if (ctrl == C_REJ0 || ctrl == C_REJ1) {
    return REJECTED;
  }

  else {
    return -1;
  }
}
