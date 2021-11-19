#include "ll.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

int writeSupervisionMessage(int port, int msg_addr, int ctrl_msg) {

  int ret = 0;
  unsigned char buf[CTRL_MSG_SIZE];
  buf[0] = FLAG;
  buf[1] = msg_addr;
  buf[2] = ctrl_msg;
  buf[3] = BCC1(SENDER_ADDR, SET);
  buf[4] = FLAG;

  ret = write(port, &buf, CTRL_MSG_SIZE);

  return ret != CTRL_MSG_SIZE ? EXIT_FAILURE : EXIT_SUCCESS;
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

    /* Ch eck if state is correct, maybe in another function */

    char buf[CTRL_MSG_SIZE];

    // Read back message and check if state is
    // correct, again, possibly in another function
    ret = read(port, &buf, CTRL_MSG_SIZE);

  } else if (role == RECEIVER) {

    /*Put read in here, and check if messsage is received correctly (state
     * machine)*/

    ret = writeSupervisionMessage(port, RECEIVER_ADDR, UA);
  }
  return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}
