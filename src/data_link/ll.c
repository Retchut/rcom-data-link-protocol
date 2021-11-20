#include "ll.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define BAUDRATE B38400

int writeSupervisionMessage(int port, int msg_addr, int ctrl_msg) {
  // TODO: Replace numbers by constants
  byte buf[CTRL_MSG_SIZE];
  buf[0] = FLAG;
  buf[1] = msg_addr;
  buf[2] = ctrl_msg;
  buf[3] = BCC1(SENDER_ADDR, SET);
  buf[4] = FLAG;

  printf("Message[0] = 0x%X\n", buf[0]);
  printf("Message[1] = 0x%X\n", buf[1]);
  printf("Message[2] = 0x%X\n", buf[2]);
  printf("Message[3] = 0x%X\n", buf[3]);
  printf("Message[4] = 0x%X\n", buf[4]);

  return write(port, &buf, CTRL_MSG_SIZE);
}

int readSupervisionMessage(int port) {
  enum state_machine { START, FLAG_RCV, A_RCV, C_RCV, BCC_OK, STOP };
  byte buf;
  size_t ret;
  byte addr, ctrl;

  enum state_machine state = START;
  /*TODO:
   * - Define struct for state machine
   * - Create event handlers for each case
   * - Create handler for state machine itself (makes code more modular)
   **/
  while (state != STOP) {

    ret = read(port, &buf, 1);

    printf("READ: 0x%X", buf);

    switch (state) {
    case START:
      if (buf == FLAG)
        state = FLAG_RCV;
      break;
    case FLAG_RCV:
      if (buf == SENDER_ADDR || buf == RECEIVER_ADDR) {
        state = A_RCV;
        addr = buf;
      } else if (buf == FLAG)
        state = FLAG_RCV;
      else
        state = START;
      break;

    case A_RCV:
      if (buf == UA || buf == SET) { // TODO: Add all possible messages
        state = C_RCV;
        ctrl = buf;
      } else if (buf == FLAG)
        state = FLAG_RCV;
      else
        state = START;

    case C_RCV:
      if (BCC1(addr, ctrl) == buf)
        state = BCC_OK;
      else if (buf == FLAG)
        state = FLAG_RCV;
      else
        state = START;

    case BCC_OK:
      if (buf == FLAG_RCV)
        state = STOP;
      else
        state = START;
    default:
      break;
    }
  }

  return CTRL_MSG_SIZE;
}

int writeInformationMessage(int port, int msg_addr, int ctrl, int information) {
  // TODO: Write this one using variable message size
  return 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* In essence this is llopen. It needs a lot of tweaks done to it, but this is
 * the overall jist of it. We need:
 * - Remove magic numbers
 * - Check if receiver or emmiter (this is the case of the emitter)
 * - Check for errors after writing, and before and after reading
 * - Create state machine for parsing the frame received
 * - Implement number of tries, as well as timeout for the sending and receiving
 *   of messages.
 * - Separate concerns into different functions and files.
 * - Create a file only for the constants mentioned above.
 */
int llopen(int port, unsigned char role) {
  int ret = 0;

  if (role == EMITTER) {

    ret = writeSupervisionMessage(port, SENDER_ADDR, SET);

    if (ret != 0 && ret != -1) {
      return FAILURE;
    }

    char buf[CTRL_MSG_SIZE];

    // Read back message and check if state is
    // correct, again, possibly in another function
    ret = read(port, &buf, CTRL_MSG_SIZE);

  } else if (role == RECEIVER) {

    /*Put read in here, and check if messsage is received correctly (state
     * machine)*/

    ret = readSupervisionMessage(port);
    ret = writeSupervisionMessage(port, RECEIVER_ADDR, UA);
  }
  return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {

  int fd, role;
  struct termios oldtio, newtio;

  fd = open(argv[1], O_RDWR | O_NOCTTY);
  if (fd < 0) {
    perror(argv[1]);
    exit(-1);
  }

  if (tcgetattr(fd, &oldtio) == -1) { /* save current port settings */
    perror("tcgetattr");
    exit(-1);
  }

  bzero(&newtio, sizeof(newtio));
  newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
  newtio.c_iflag = IGNPAR;
  newtio.c_oflag = 0;

  /* set input mode (non-canonical, no echo,...) */
  newtio.c_lflag = 0;

  newtio.c_cc[VTIME] = 30; /* inter-character timer unused */
  newtio.c_cc[VMIN] = 0;   /* blocking read until 1 chars received */

  /*
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a
    leitura do(s) próximo(s) caracter(es)
  */

  tcflush(fd, TCIOFLUSH);

  if (tcsetattr(fd, TCSANOW, &newtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  if (!strcmp(argv[2], "receiver")) {
    printf("Initialized as receiver!\n");
    role = RECEIVER;
  } else if (!strcmp(argv[2], "emitter")) {
    printf("Initialized as emitter\n");
    role = EMITTER;
  }

  llopen(fd, role);

  if (tcsetattr(fd, TCSANOW, &oldtio) == -1) {
    perror("tcsetattr");
    exit(-1);
  }

  close(fd);

  return 0;
}
