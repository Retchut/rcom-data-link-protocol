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

struct termios oldtio;

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

int readSupervisionFrame(int fd) {

  enum state_machine { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP };

  enum state_machine st = START;
  int ret = 0;
  unsigned char buf;
  int addr, ctrl;

  time_t start_time = time(NULL);
  time_t end_time = time(&start_time);

  while (difftime(end_time, start_time) < 3) {

    ret = read(fd, &buf, 1);

    if (ret == -1) {
      perror("Error in read\n");
      return -1;
    }

    if (!strcmp(&buf, "")) {
      end_time = time(NULL);
      continue;
    } else {
      printf("Read %c from buf\n", buf);
      end_time = time(&start_time);
      printf("Start time is now %ld\n", start_time);
    }

    switch (st) {
    case START:
      switch (buf) {
      case FLAG:
        st = FLAG_RCV;
        break;
      default:
        break;
      }
      break;
    case FLAG_RCV:
      switch (buf) {
      case A_SEND_CMD_ADDR:
      case A_RECV_CMD_ADDR:
        st = A_RCV;
        addr = buf;
      case FLAG:
        break;
      default:
        st = START;
        break;
      }
      break;
    case A_RCV:
      switch (buf) {
      // TODO: Implement all the other possible Control_messages
      case C_SET:
      case C_UA:
      case C_DISC:
        st = C_RCV;
        ctrl = buf;
        break;
      case FLAG:
        st = FLAG_RCV;
        break;
      default:
        st = START;
        break;
      }
      break;
    case C_RCV:
      switch (buf) {
      case FLAG:
        st = FLAG_RCV;
        break;
      default:
        if ((BCC1(addr, ctrl)) == buf) {
          st = BCC_OK;
        } else {
          st = START;
        }
        break;
      }
      break;
    case BCC_OK:
      switch (buf) {
      case FLAG:
        st = STOP;
        break;
      default:
        st = START;
        break;
      }
      break;
    case STOP:
      printf("Succesfully delivered frame!\n");
      return 0;
    }
  }

  printf("Ups, a timeout occured\n");
  return -1;
}

bool testFrameEquality(unsigned char *frame1, unsigned char *frame2,
                       size_t size) {
  for (size_t p = 0; p < 0; p++) {
    if (frame1[p] != frame2[p])
      return false;
  }
  return true;
}

int writeSupervisionFrame(int fd, int msg_addr, int msg_ctrl) {
  unsigned char buf[SU_FRAME_SIZE];

  buf[0] = FLAG;
  buf[1] = msg_addr;
  buf[2] = msg_ctrl;
  buf[3] = BCC1(msg_addr, msg_ctrl);
  buf[4] = FLAG;

  return write(fd, &buf, SU_FRAME_SIZE) != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}

int llopen(int port, bool transmitter) {
  int fd;
  struct termios newtio;

  char serialPort[11];
  sprintf(serialPort, "%s%d", "/dev/ttyS", port);

  printf("%s\n", serialPort);

  // open serial port
  fd = open(serialPort, O_RDWR | O_NOCTTY);

  if (fd < 0) {
    perror(serialPort);
    return (-1);
  }

  // save original serial port settings
  if (tcgetattr(fd, &oldtio) == -1) {
    perror("tcgetattr");
    return (-1);
  }

  // prepares new serial port settings
  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;
  newtio.c_lflag = 0;     // input mode (non-canonical, no echo,...)
  newtio.c_cc[VTIME] = 1; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 0;  /* blocking read until 5 chars received */

  tcflush(fd, TCIOFLUSH); // discards data written to fd

  // apply new serial port settings
  if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
    perror("tcsetattr");
    return (-1);
  }
  printf("New termios structure set\n");

  // build frames
  // SET frame
  /*
  unsigned char setFrame[SU_FRAME_SIZE];
  if (buildFrame(setFrame, A_SEND_CMD_ADDR, C_SET, NULL, 0) != 0) {
    printf("Error building frame - SET frame.\n");
    return -1;
  }
  // UA frame
  unsigned char uaFrame[SU_FRAME_SIZE];
  if (buildFrame(uaFrame, A_RECV_RESP_ADDR, C_UA, NULL, 0) != 0) {
    printf("Error building frame - UA response frame.\n");
    return -1;
  }
  */

  bool completed = false;

  if (transmitter) {
    // build test frame
    /*
    unsigned char *expectedUA = (unsigned char *)malloc(SU_FRAME_SIZE);
    if (setFrame == NULL) {
      printf("Error in malloc - ua test frame.\n");
      return -1;
    }
    */

    // TODO: implement timeout here
    // while (!completed) {
    // write command frame
    if (writeSupervisionFrame(fd, A_SEND_CMD_ADDR,
                              C_SET) /*writeFrame(fd, setFrame, SU_FRAME_SIZE)*/
        != 0) {
      printf("Error writing set frame.\n");
      // continue;
    }
    printf("Sent SET frame.\n");

    // read response frame
    if (readSupervisionFrame(fd) /*readFrame(fd, expectedUA, SU_FRAME_SIZE)*/
        != 0) {
      printf("Error reading UA frame.\n");
      // continue;
    }

    // test expected response
    /*
    if (!testFrameEquality(expectedUA, uaFrame, SU_FRAME_SIZE)) {
      printf("SET response not UA frame.\n");
      continue;
    }
    */
    // printf("Received UA frame.\n");
    completed = true;
    //}

    // free(expectedUA);
  } else {
    // build test frame
    /*unsigned char *expectedSet = (unsigned char *)malloc(SU_FRAME_SIZE);
    if (setFrame == NULL) {
      printf("Error in malloc - SET frame.\n");
      return -1;
    }
    */

    // TODO: implement timeout here
    // while (!completed) {
    // read response frame
    if (readSupervisionFrame(fd) /*readFrame(fd, expectedSet, SU_FRAME_SIZE)*/
        != 0) {
      printf("Error reading UA frame.\n");
      //   continue;
    }

    // test expected response
    /*
    if (!testFrameEquality(expectedSet, uaFrame, SU_FRAME_SIZE)) {
      printf("SET response not UA frame.\n");
      continue;
    }
    */
    // printf("Received SET frame.\n");

    // write command frame
    if (writeSupervisionFrame(fd, A_RECV_CMD_ADDR,
                              C_UA) /*writeFrame(fd, uaFrame, SU_FRAME_SIZE)*/
        != 0) {
      printf("Error writing set frame.\n");
      //  continue;
    }
    // printf("Sent UA frame.\n");
    completed = true;
    //}

    //    free(expectedSet);
  }

  // free(setFrame);
  // free(uaFrame);

  return fd;
}

int llclose(int fd) {
  // restore original serial port settings
  if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
    perror("tcsetattr");
    return (-1);
  }

  // close serial port
  if (close(fd) == -1) {
    perror("close");
    return (-1);
  }

  return 0;
}
