#include "ll.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

/* In essence this is llopen. It needs a lot of tweaks done to it, but this is
 * the overall jist of it. We need:
 * - Remove magic numbers
 * - Check if receiver or emmiter (this is the case of the emitter)
 * - Check for errors after writing, and before and after reading
 * - Implement number of tries, as well as timeout for the sending and receiving
 *   of messages.
 * - Separate concerns into different functions and files.
 * - Create a file only for the constants mentioned above.
 */
int llopen(int port, unsigned char role) {
  int ret;
  char buf[5];
  ret = write(port, &buf, 5);
  ret = read(port, &buf, 5);
  return 1;
}
